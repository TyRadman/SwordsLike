#include "BaseParryComponent.h"

#include "BaseEntityData.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerState.h"
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

	SetIsReplicatedByDefault(true);
}

void UBaseParryComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBaseParryComponent, CurrentPosture);
	DOREPLIFETIME(UBaseParryComponent, MaxPosture);
	DOREPLIFETIME(UBaseParryComponent, CurrentParryState);
	DOREPLIFETIME(UBaseParryComponent, CurrentAttacker);
	DOREPLIFETIME(UBaseParryComponent, CurrentCombatState);
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

	PlayerCharacter = CustomCharacter;
	
	if(PlayerCharacter->GetController())
	{
		PrintOnScreen(FString::Printf(TEXT("FOUND CONTROLLER ON %s"), *UEnum::GetValueAsString(GetOwnerRole())), FColor::Green, 6.0f);
		OwnerController = PlayerCharacter->GetController();
	}
	else
	{
		PrintOnScreen(FString::Printf(TEXT("ERROR: NOT FOUND CONTROLLER ON %s"), *UEnum::GetValueAsString(GetOwnerRole())), FColor::Red, 6.0f);
	}
	
	AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();

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
	
	if(GetOwnerRole() != ROLE_AutonomousProxy)
	{
		if(UOverheadHealthBarWidget* OverheadWidget = CustomCharacter->GetOverHeadHUDComponent())
		{
			OnPostureChanged.AddUObject(OverheadWidget, &UOverheadHealthBarWidget::SetPostureOverheadBarValue);
		}
		else
		{
			PrintOnScreen_Local(TEXT("No overhead widget"));
		}
	}

	///////////
	/// Initialization
	///////////
	SetMaxPosture(CustomCharacter->GetPlayerStats()->MaxPosture);
	FullyRefillPosture();
	StunRecoveryTime = KnockDownMontage->GetPlayLength();
}

void UBaseParryComponent::CacheValues()
{
		PrintOnScreen(TEXT("CACHE CALLED"));
	ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(GetOwner());
	if(!CustomCharacter)
	{
		PrintOnScreen_Local(TEXT("No Character passed to the Parry Component"));
		return;
	}

	PlayerCharacter = CustomCharacter;
	
	if(PlayerCharacter->GetController())
	{
		PrintOnScreen(FString::Printf(TEXT("FOUND CONTROLLER ON %s"), *UEnum::GetValueAsString(GetOwnerRole())), FColor::Green, 6.0f);
		OwnerController = PlayerCharacter->GetController();
	}
	else
	{
		PrintOnScreen(FString::Printf(TEXT("ERROR: NOT FOUND CONTROLLER ON %s"), *UEnum::GetValueAsString(GetOwnerRole())), FColor::Red, 6.0f);
	}
	
	AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();
}

void UBaseParryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// PrintOnScreen_Local(1, FString::Printf(TEXT("%s"), *UEnum::GetValueAsString(CurrentCombatState)));
	// PlayerCharacter->PrintOverhead(*UEnum::GetValueAsString(CurrentCombatState));
	
	if(bCanRecoverPosture && CurrentPosture < MaxPosture && CurrentCombatState == ECombatState::Normal)
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

	const ASwordslikeCharacter* AttackerCharacter = Cast<ASwordslikeCharacter>(AttackSource);

	// check if the attack is caused by another character, otherwise, no parry takes place
	if(!AttackerCharacter)
	{
		CurrentParryState = EParryState::None;
	}
	
	// check if the damage instigate is within the parry range
	const FVector OwnerForward = PlayerCharacter->GetActorForwardVector();
	const FVector DirectionToAttacker = (AttackSource->GetActorLocation() - PlayerCharacter->GetActorLocation()).GetSafeNormal();

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

void UBaseParryComponent::SetParryState(const EParryState State)
{
	CurrentParryState = State;
}

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
	CurrentAttacker = DamageInfo.DamageInstigatorCharacter;
	
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
			OnParrySuccessful_Local.Broadcast(CurrentParryState);
		}
	}
}

void UBaseParryComponent::AddToCurrentPosture(const float Amount)
{
	if (!HasAuthority())
	{
		if(GetOwnerRole() == ROLE_AutonomousProxy)
		{
			Server_AddToCurrentPosture(Amount);
		}
	}
	else
	{
		PerformAddToCurrentPosture(Amount);
		OnRep_CurrentPosture();
	}
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

		if(HasAuthority())
		{
			if(CurrentCombatState == ECombatState::Normal)
			{
				OnStunned();
			}
			else if(CurrentCombatState == ECombatState::Stunned)
			{
				OnKnockDown();
			}
		}
	}
	
	if(OnPostureChanged.IsBound())
	{
		OnPostureChanged.Broadcast(CurrentPosture, MaxPosture);
	}
}

void UBaseParryComponent::OnRep_CurrentCombatState()
{
}

void UBaseParryComponent::OnStunned()
{
	if(!HasAuthority())
	{
		Server_PerformStun();
	}
	else
	{
		PerformStun();
		Multicast_PerformStun();
	}
}

void UBaseParryComponent::Server_PerformStun_Implementation()
{
	PerformStun();
	Multicast_PerformStun();
}

void UBaseParryComponent::Multicast_PerformStun_Implementation()
{
	if(!HasAuthority())
	{
		AnimInstance->Montage_Play(StunMontage);
	}
}

void UBaseParryComponent::PerformStun()
{
	CurrentCombatState = ECombatState::Stunned;
	AnimInstance->Montage_Play(StunMontage);
	bCanRecoverPosture = false;
}

void UBaseParryComponent::OnRecoverFromStun()
{
	if(!HasAuthority())
	{
		Server_PerformRecoverFromStun();
	}
	else
	{
		PerformRecoverFromStun();
	}
}

void UBaseParryComponent::Server_PerformRecoverFromStun_Implementation()
{
	PerformRecoverFromStun();
}

void UBaseParryComponent::PerformRecoverFromStun()
{
	CurrentCombatState = ECombatState::Normal;
	bCanRecoverPosture = true;

	FullyRefillPosture();
}

void UBaseParryComponent::OnKnockDown()
{
	if (HasAuthority())
	{
		PerformKnockDown();
		Multicast_PerformKnockDown();
	}
	else
	{
		Server_PerformKnockDown();
	}
}

void UBaseParryComponent::Server_PerformKnockDown_Implementation()
{
	PerformKnockDown();
	Multicast_PerformKnockDown();
}

void UBaseParryComponent::Multicast_PerformKnockDown_Implementation()
{
	if(HasAuthority())
	{
		return;
	}
	
	if (!CurrentAttacker || !PlayerCharacter || !AnimInstance)
	{
		return;
	}
	
	const FVector Direction = (CurrentAttacker->GetActorLocation() - PlayerCharacter->GetActorLocation()).GetSafeNormal();
	const FRotator HitRotation = Direction.Rotation();
	
	if(OwnerController)
	{
		OwnerController->SetControlRotation(HitRotation);
	}
	
	PlayerCharacter->SetActorRotation(HitRotation);
	PlayerCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;
	PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;

	AnimInstance->Montage_Play(KnockDownMontage, 1.f);
}

void UBaseParryComponent::PerformKnockDown()
{
	CurrentCombatState = ECombatState::KnockedDown;
	
	if (!CurrentAttacker || !PlayerCharacter || !AnimInstance)
	{
		return;
	}
	
	const FVector Direction = (CurrentAttacker->GetActorLocation() - PlayerCharacter->GetActorLocation()).GetSafeNormal();
	const FRotator HitRotation = Direction.Rotation();
	
	if(OwnerController)
	{
		OwnerController->SetControlRotation(HitRotation);
	}

	PlayerCharacter->SetActorRotation(HitRotation);
	PlayerCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;
	PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;

	AnimInstance->Montage_Play(KnockDownMontage, 1.f);
}

void UBaseParryComponent::StartRecoveryFromKnockDown()
{
	
}

void UBaseParryComponent::EndRecoveryFromKnockDown()
{
	CurrentCombatState = ECombatState::Normal;
	PlayerCharacter->RestoreCharacterRotation();
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
