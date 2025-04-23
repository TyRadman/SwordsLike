#include "BaseParryComponent.h"

#include "BaseEntityAnimationsComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/PlayerCombatComponent.h"
#include "Player/PlayerStartCharacterDataAsset.h"
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
	// DOREPLIFETIME(UBaseParryComponent, bCanRecoverPosture);
	// DOREPLIFETIME(UBaseParryComponent, CurrentCombatState);
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

	bIsLocallyControlled = CustomCharacter->IsLocallyControlled();

	PlayerCharacter = CustomCharacter;
	
	if(PlayerCharacter->GetController())
	{
		// TODO: Remove the controller if not need (hopefully, we don't)
		OwnerController = PlayerCharacter->GetController();
	}
	else
	{
		// PrintOnScreen(FString::Printf(TEXT("ERROR: NOT FOUND CONTROLLER ON %s"), *UEnum::GetValueAsString(GetOwnerRole())), FColor::Red, 6.0f);
	}
	
	AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();

	if(PlayerCharacter->GetAnimation())
	{
		AnimationComponent = PlayerCharacter->GetAnimation();
	}
	else
	{
		PrintOnScreen_Local(TEXT("No Animation"));
	}

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
	if(const UPlayerStartCharacterDataAsset* Data = CustomCharacter->GetData())
	{
		const float MaxStartingPosture = Data->StartingPosture;
		
		SetMaxPosture(MaxStartingPosture);
		FullyRefillPosture();
		StunRecoveryTime = KnockDownMontage->GetPlayLength();
	}
}

void UBaseParryComponent::CacheValues()
{
	ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(GetOwner());
	if(!CustomCharacter)
	{
		return;
	}

	PlayerCharacter = CustomCharacter;
	
	if(PlayerCharacter->GetController())
	{
		OwnerController = PlayerCharacter->GetController();
	}
	
	AnimInstance = PlayerCharacter->GetMesh()->GetAnimInstance();
}

void UBaseParryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!bIsLocallyControlled)
	{
		return;
	}
	
	if(bCanRecoverPosture && CurrentPosture < MaxPosture && CurrentCombatState == ECombatState::Normal)
	{
		// TODO: fix this with overloads or call something else
		RecoverDamageInfo.PostureDamage = -PostureRecoveryRate * DeltaTime;
		AddToCurrentPosture(RecoverDamageInfo);
	}
}

#pragma region Parry Network
void UBaseParryComponent::Parry()
{
	// Not used
	// if(OnParryStartedEvent.IsBound())
	// {
	// 	OnParryStartedEvent.Broadcast();
	// }
	
	if(!HasAuthority())
	{
		Server_Parry();
	}
	else
	{
		bIsParrying = true;
		AnimInstance->Montage_Play(ParryMontage);
		AnimInstance->Montage_SetNextSection(StartSectionName, MiddleSectionName);
	}
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

// Always called through the client
void UBaseParryComponent::SetParryState(const EParryState State)
{
	if(!HasAuthority())
	{
		Server_SetParryState(State);
	}
	else
	{
		PerformSetParryState(State);
	}
}

void UBaseParryComponent::Server_SetParryState_Implementation(const EParryState State)
{
	PerformSetParryState(State);
}

void UBaseParryComponent::PerformSetParryState(const EParryState State)
{
	CurrentParryState = State;
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

	if(!DamageInfo.DamageInstigator)
	{
		PrintOnScreen(TEXT("ValidateParry: No attack in attack info"));
		return EParryState::None;
	}
	
	AActor* AttackSource = DamageInfo.DamageInstigator;

	ASwordslikeCharacter* AttackerCharacter = Cast<ASwordslikeCharacter>(AttackSource);

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
		if(!HasAuthority())
		{
			Server_InterruptAttacker(AttackerCharacter);
		}
		else
		{
			AttackerCharacter->GetCombatComponent()->ForceStopAttack(true);
		}
	}
	
	return CurrentParryState;
}

void UBaseParryComponent::Server_InterruptAttacker_Implementation(ASwordslikeCharacter* Attacker)
{
	Attacker->GetCombatComponent()->ForceStopAttack(true);
}

#pragma region Posture
/**
 * Is triggered when the posture takes damage whether by parried attacks or by no parried attacks. NOTE: perfect parries will not trigger any logic here.
 * So far, only called on the client.
 * @param DamageInfo 
 */
void UBaseParryComponent::DamagePosture(const FDamageInfo DamageInfo)
{
	// OnParry();
	
	if(bIsLocallyControlled)
	{
		PerformDamagePosture(DamageInfo);
	}
	else
	{
		Server_DamagePosture(DamageInfo);
		// PerformDamagePosture(DamageInfo);
	}
}

void UBaseParryComponent::Server_DamagePosture_Implementation(const FDamageInfo DamageInfo)
{
	Client_DamagePosture(DamageInfo);
}

void UBaseParryComponent::Client_DamagePosture_Implementation(const FDamageInfo DamageInfo)
{
	PerformDamagePosture(DamageInfo);
}

// This is only called through the client
void UBaseParryComponent::PerformDamagePosture(const FDamageInfo DamageInfo)
{
	if(CurrentParryState != EParryState::Perfect)
	{
		AddToCurrentPosture(DamageInfo);

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

void UBaseParryComponent::AddToCurrentPosture(const FDamageInfo DamageInfo)
{
	if (!HasAuthority())
	{
		Server_AddToCurrentPosture(DamageInfo, CurrentCombatState);
	}
	else
	{
		PerformServerAddToCurrentPosture(DamageInfo, CurrentCombatState);
	}
}

void UBaseParryComponent::Server_AddToCurrentPosture_Implementation(const FDamageInfo DamageInfo, const ECombatState State)
{
	PerformServerAddToCurrentPosture(DamageInfo, State);
}

void UBaseParryComponent::PerformServerAddToCurrentPosture(const FDamageInfo DamageInfo, const ECombatState State)
{
	const float PostureDamage = -DamageInfo.PostureDamage * PostureMultipliers[CurrentParryState];
	PerformAddToCurrentPosture(PostureDamage);
	OnRep_CurrentPosture();
		
	if(CurrentPosture <= 0)
	{
		// PrintOnScreen(FString::Printf(TEXT("%s [%s]"), *UEnum::GetValueAsString(State), *UEnum::GetValueAsString(GetOwnerRole())));
		if(State == ECombatState::Normal)
		{
			OnStunned();
		}
		else if(State == ECombatState::Stunned)
		{
			// TODO: must be handled better. A cheat for now.
			// CurrentAttacker = DamageInfo.DamageInstigator;
			OnKnockDown(DamageInfo.DamageInstigator);
		}
	}
	else if(PostureDamage < 0 && CurrentParryState == EParryState::None)
	{
		AnimationComponent->PlayHitReactMontage(DamageInfo);
	}
}

void UBaseParryComponent::PerformAddToCurrentPosture(const float Amount)
{
	CurrentPosture = FMath::Clamp(CurrentPosture + Amount, 0.f, MaxPosture);
}

void UBaseParryComponent::OnRep_CurrentPosture()
{
	// Update HUDs locally and remotely (overhead)
	if(OnPostureChanged.IsBound())
	{
		OnPostureChanged.Broadcast(CurrentPosture, MaxPosture);
	}
}

// Called only through the server
void UBaseParryComponent::OnStunned()
{
	PerformStun();
}

void UBaseParryComponent::PerformStun()
{
	PlayerCharacter->PerformCameraShake(StunCameraShake);
	SetCombatState(ECombatState::Stunned);

	if(!AnimationComponent)
	{
		PrintOnScreen(TEXT("AnimationComponent is invalid"));
	}
	if(!StunMontage)
	{
		PrintOnScreen(TEXT("StunMontage is invalid"));
	}
	
	AnimationComponent->PlayMontage(StunMontage, true);
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
	SetCombatState(ECombatState::Normal);
	bCanRecoverPosture = true;
	FullyRefillPosture();
}

// Called only through the server
void UBaseParryComponent::OnKnockDown(AActor* Attacker)
{
	// if (HasAuthority())
	// {
	// PlayerCharacter->PerformCameraShake(KnockDownCameraShake);
	PrintOnScreen(TEXT("Knockdown"));
	SetCombatState(ECombatState::KnockedDown);
	PerformKnockDown(Attacker);
	Multicast_PerformKnockDown(Attacker);
	// }
	// else
	// {
	// 	Server_PerformKnockDown();
	// }
}

void UBaseParryComponent::Server_PerformKnockDown_Implementation()
{
	// SetCombatState(ECombatState::KnockedDown);
	// PerformKnockDown();
	// Multicast_PerformKnockDown();
}

void UBaseParryComponent::Multicast_PerformKnockDown_Implementation(AActor* Attacker)
{
	if(HasAuthority())
	{
		return;
	}

	PerformKnockDown(Attacker);
}

void UBaseParryComponent::PerformKnockDown(const AActor* Attacker)
{
	if (!Attacker || !PlayerCharacter || !AnimInstance)
	{
		PrintOnScreen(TEXT("Something is missing"));
		return;
	}
	
	const FVector Direction = (Attacker->GetActorLocation() - PlayerCharacter->GetActorLocation()).GetSafeNormal();
	const FRotator HitRotation = Direction.Rotation();
	
	// if(OwnerController)
	// {
	// 	OwnerController->SetControlRotation(HitRotation);
	// }

	PlayerCharacter->SetActorRotation(HitRotation);
	PlayerCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = true;
	PlayerCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;

	PrintOnScreen(TEXT("Playing knockdown montage"));
	AnimInstance->Montage_Play(KnockDownMontage);
}

void UBaseParryComponent::StartRecoveryFromKnockDown()
{
	
}

void UBaseParryComponent::EndRecoveryFromKnockDown()
{
	SetCombatState(ECombatState::Normal);
	PlayerCharacter->RestoreCharacterRotation();
	FullyRefillPosture();
}

void UBaseParryComponent::SetCombatState(const ECombatState State)
{
	if(bIsLocallyControlled)
	{
		CurrentCombatState = State;
	}
	else
	{
		Client_SetCombatState(State);
	}
}

void UBaseParryComponent::Client_SetCombatState_Implementation(const ECombatState State)
{
	CurrentCombatState = State;
}

void UBaseParryComponent::SetMaxPosture(const float NewAmount)
{
	MaxPosture = NewAmount;
}

void UBaseParryComponent::FullyRefillPosture()
{
	RecoverDamageInfo.PostureDamage = -MaxPosture;
	AddToCurrentPosture(RecoverDamageInfo);
}
#pragma endregion 

void UBaseParryComponent::PlayParryEffects(const FDamageInfo DamageInfo)
{
	OnParry();
	
	if(!HasAuthority())
	{
		Server_PlayParryEffects(DamageInfo);
	}
	else
	{
		Multicast_PlayParryEffects(DamageInfo);
	}
}

void UBaseParryComponent::Server_PlayParryEffects_Implementation(const FDamageInfo DamageInfo)
{
	Multicast_PlayParryEffects(DamageInfo);
}

void UBaseParryComponent::Multicast_PlayParryEffects_Implementation(const FDamageInfo DamageInfo)
{
	if(GetOwnerRole() == ROLE_AutonomousProxy)
	{
		return;
	}
	
	OnParry();
}

void UBaseParryComponent::OnParry()
{
	// play parry particles if there is a parry
	if(CurrentParryState != EParryState::None)
	{
		if(OnParrySuccessful_Local.IsBound())
		{
			OnParrySuccessful_Local.Broadcast(CurrentParryState);
		}
	}
}