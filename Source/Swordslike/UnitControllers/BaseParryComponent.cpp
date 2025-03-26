#include "BaseParryComponent.h"

#include "BaseEntityData.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerCombatComponent.h"
#include "Player/SwordslikeCharacter.h"
#include "Swordslike/UI/HUD/MasterHUD.h"
#include "Swordslike/UI/HUD/HealthBars/PlayerHealthBar.h"
#include "Swordslike/UI/WorldUIElements/OverheadHealthBarWidget.h"

class UPlayerHealthBar;

UBaseParryComponent::UBaseParryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBaseParryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBaseParryComponent, CurrentPosture);
	DOREPLIFETIME(UBaseParryComponent, MaxPosture);
	DOREPLIFETIME(UBaseParryComponent, CurrentParryState);
}

void UBaseParryComponent::InitEntityComponent(ACharacter* Character)
{
	if(!Character)
	{
		PrintOnScreen_Local(TEXT("No Character passed to the Parry Component"));
		return;
	}

	ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Character);
	if(!CustomCharacter)
	{
		PrintOnScreen_Local(TEXT("No Character passed to the Parry Component"));
		return;
	}

	OwnerCharacter = Character;
	AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();

	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UBaseParryComponent::OnParryNotifyStart);
	OnKnockedDown.AddUObject(CustomCharacter, &ASwordslikeCharacter::OnKnockedDown);
	OnKnockedDownRecover.AddUObject(CustomCharacter, &ASwordslikeCharacter::OnKnockedDownRecover);
	OnParrySuccessful_Local.AddUObject(CustomCharacter, &ASwordslikeCharacter::OnAttackParried);
	
	//////////
	/// UI
	//////////
	if(GetOwnerRole() == ROLE_AutonomousProxy || HasAuthority())
	{
		if(const UMasterHUD* MasterHUD = CustomCharacter->GetMasterHUD())
		{
			if(UPlayerHealthBar* PlayerHUD = MasterHUD->GetStatsHUD())
			{
				OnPostureChanged.AddUObject(PlayerHUD, &UPlayerHealthBar::SetPostureBarValue);
				PlayerHUD->SetPostureBarValue(1.f, 1.f);
			}
		}
	}
	
	if(GetOwnerRole() == ROLE_SimulatedProxy)
	{
		if(UOverheadHealthBarWidget* OverheadWidget = CustomCharacter->GetOverHeadHUDComponent())
		{
			OnPostureChanged.AddUObject(OverheadWidget, &UOverheadHealthBarWidget::SetPostureOverheadBarValue);
		}
	}

	///////////
	/// Initialization
	///////////
	SetMaxPosture(CustomCharacter->GetPlayerStats()->MaxPosture);
	FullyRefillPosture();
	KnockDownRecoveryTime = KnockDownMontage->GetPlayLength();
}

void UBaseParryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(bCanRecoverPosture && CurrentPosture < MaxPosture)
	{
		AddToCurrentPosture(PostureRecoveryRate * DeltaTime);
	}
}

#pragma region Parry Network
void UBaseParryComponent::Parry()
{
	if(OnParryStartedEvent.IsBound())
	{
		OnParryStartedEvent.Broadcast();
	}
	
	if(!HasAuthority())
	{
		Server_Parry();
		return;
	}

	bIsParrying = true;
	AnimInstance->Montage_Play(ParryMontage);
	AnimInstance->Montage_SetNextSection(StartSectionName, MiddleSectionName);
}

void UBaseParryComponent::Server_Parry_Implementation()
{
	Multicast_Parry();
}

void UBaseParryComponent::Multicast_Parry_Implementation()
{
	bIsParrying = true;
	AnimInstance->Montage_Play(ParryMontage);
	AnimInstance->Montage_SetNextSection(StartSectionName, MiddleSectionName);
}
#pragma endregion

#pragma region End Parry Network
void UBaseParryComponent::EndParry()
{
	if(OnParryEndedEvent.IsBound())
	{
		OnParryEndedEvent.Broadcast();
	}

	if(!HasAuthority())
	{
		Server_EndParry();
	}
	
	bIsParrying = false;
	CurrentParryState = EParryState::None;
	AnimInstance->Montage_JumpToSection(EndSectionName, ParryMontage);
	AnimInstance->Montage_Stop(0.3f, ParryMontage);
}

void UBaseParryComponent::Server_EndParry_Implementation()
{
	Multicast_EndParry();
}

void UBaseParryComponent::Multicast_EndParry_Implementation()
{
	CurrentParryState = EParryState::None;
	AnimInstance->Montage_JumpToSection(EndSectionName, ParryMontage);
	AnimInstance->Montage_Stop(0.3f, ParryMontage);
}
#pragma endregion

EParryState UBaseParryComponent::ValidateParry(const FDamageInfo& DamageInfo)
{
	// TODO: should be moved somewhere else
	if(CurrentParryState == EParryState::Normal)
	{
		AnimInstance->Montage_JumpToSection(HitSectionName);
	}
	
	AActor* AttackSource = DamageInfo.DamageInstigator;

	ASwordslikeCharacter* AttackerCharacter = Cast<ASwordslikeCharacter>(AttackSource);

	// check if the attack is caused by another character, otherwise, no parry takes place
	if(!AttackerCharacter)
	{
		CurrentParryState = EParryState::None;
	}
	
	// check if the damage instigate is within the parry range
	const FVector OwnerForward = OwnerCharacter->GetActorForwardVector();
	const FVector DirectionToAttacker = (AttackSource->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();

	if(const float DotProduct = FVector::DotProduct(OwnerForward, DirectionToAttacker); DotProduct < 0.5f)
	{
		CurrentParryState =  EParryState::None;
	}

	if(CurrentParryState == EParryState::Perfect)
	{
		AttackerCharacter->GetCombatComponent()->ForceStopAttack();
	}
	
	return CurrentParryState;
}

#pragma region Notifies
void UBaseParryComponent::OnParryNotifyStart(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if(!ParryStates.Contains(NotifyName))
	{
		return;
	}
	
	CurrentParryState = ParryStates[NotifyName];
	// PrintOnScreen(FString::Printf(TEXT("Parry state %s"), *UEnum::GetValueAsString(CurrentParryState)));
}
#pragma endregion

#pragma region Posture
/**
 * Is triggered when the posture takes damage whether by parried attacks or by no parried attacks. NOTE: perfect parries will not trigger any logic here.
 * @param DamageInfo 
 */
void UBaseParryComponent::DamagePosture(const FDamageInfo DamageInfo)
{
	OnParry();
	
	if(!HasAuthority())
	{
		Server_DamagePosture(DamageInfo);
	}
	else
	{
		Multicast_DamagePosture(DamageInfo);
		PerformDamagePosture(DamageInfo);
	}
}

void UBaseParryComponent::Server_DamagePosture_Implementation(const FDamageInfo DamageInfo)
{
	Multicast_DamagePosture(DamageInfo);
	PerformDamagePosture(DamageInfo);
}

void UBaseParryComponent::Multicast_DamagePosture_Implementation(const FDamageInfo DamageInfo)
{
	if(IsAutonomousProxy())
	{
		return;
	}
	
	OnParry();
}

void UBaseParryComponent::PerformDamagePosture(const FDamageInfo DamageInfo)
{
	if(CurrentParryState != EParryState::Perfect)
	{
		const float Multiplier = PostureMultipliers[CurrentParryState];
		AddToCurrentPosture(-DamageInfo.PostureDamage * Multiplier);

		// disable and enable posture recovery
		if(GetWorld()->GetTimerManager().IsTimerActive(PostureRecoveryTimerHandle))
		{
			GetWorld()->GetTimerManager().ClearTimer(PostureRecoveryTimerHandle);
		}

		bCanRecoverPosture = false;

		GetWorld()->GetTimerManager().SetTimer(
			PostureRecoveryTimerHandle,
			[this]()
			{
				bCanRecoverPosture = true;
			},
		DelayBeforePostureRecovery,
		false);
	}
}

void UBaseParryComponent::OnParry()
{
	// play parry particles
	if(CurrentParryState != EParryState::None)
	{
		if(OnParrySuccessful_Local.IsBound())
		{
			OnParrySuccessful_Local.Broadcast();
		}
	}
}

void UBaseParryComponent::AddToCurrentPosture(const float Amount)
{
	// PrintOnScreen_Local(FString::Printf(TEXT("AddToCurrentPosture: %f"), CurrentPosture));
	if (!HasAuthority())
	{
		Server_AddToCurrentPosture(Amount);
	}
	else
	{
		PerformAddToCurrentPosture(Amount);
		OnRep_CurrentPosture();
	}

	// Update Posture only on the server
	// CurrentPosture = FMath::Clamp(CurrentPosture + Amount, 0.f, MaxPosture);
	// PrintOnScreen_Local(FString::Printf(TEXT("AddToCurrentPosture: %f"), CurrentPosture));

}

void UBaseParryComponent::Server_AddToCurrentPosture_Implementation(const float Amount)
{
	PerformAddToCurrentPosture(Amount);
	OnRep_CurrentPosture();
}

void UBaseParryComponent::PerformAddToCurrentPosture(const float Amount)
{
	CurrentPosture = FMath::Min(CurrentPosture + Amount, MaxPosture);
}

void UBaseParryComponent::OnRep_CurrentPosture()
{
	// on posture broken
	if(CurrentPosture <= 0)
	{
		CurrentPosture = 0.f;
		PerformKnockDown();
	}
	
	if(OnPostureChanged.IsBound())
	{
		OnPostureChanged.Broadcast(CurrentPosture, MaxPosture);
	}
}

void UBaseParryComponent::OnRep_MaxPosture()
{
	
}

void UBaseParryComponent::OnRep_CurrentParryState()
{
	
}

void UBaseParryComponent::PerformKnockDown()
{
	PrintOnScreen_Local(TEXT("KnockDown"));
	
	AnimInstance->Montage_Play(KnockDownMontage, 1.0f, EMontagePlayReturnType::MontageLength, 0.f, true);
	bIsKnockedDown = true;
	bCanRecoverPosture = false;
	
	if(GetWorld()->GetTimerManager().IsTimerActive(PostureRecoveryTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(PostureRecoveryTimerHandle);
	}

	GetWorld()->GetTimerManager().SetTimer(
		KnockDownRecoveryTimerHandle,
		this,
		&UBaseParryComponent::RecoverFromKnockDown,
	KnockDownRecoveryTime,
	false);

	UE_LOG(LogTemp, Display, TEXT("Knocked down-1"));
	
	if(OnKnockedDown.IsBound())
	{
	UE_LOG(LogTemp, Display, TEXT("Knocked down0"));
		OnKnockedDown.Broadcast();
	}
}

void UBaseParryComponent::RecoverFromKnockDown()
{
	PrintOnScreen_Local(TEXT("Recovery from knockdown"));
	bIsKnockedDown = false;
	bCanRecoverPosture = true;

	FullyRefillPosture();
	
	if(OnKnockedDownRecover.IsBound())
	{
		OnKnockedDownRecover.Broadcast();
	}
}

void UBaseParryComponent::SetMaxPosture(const float NewAmount)
{
	MaxPosture = NewAmount;
}

void UBaseParryComponent::FullyRefillPosture()
{
	AddToCurrentPosture(MaxPosture);
}
#pragma endregion 
