#include "BaseCombatComponent.h"

#include "BaseEntityAnimationsComponent.h"
#include "Common/WeaponHandlerComponent.h"
#include "GameFramework/Character.h"
#include "Player/SwordslikeCharacter.h"
#include "Swordslike/UI/WorldUIElements/WeaponAttackIndicatorWidget.h"

DEFINE_LOG_CATEGORY(CombatComponent);

UBaseCombatComponent::UBaseCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UBaseCombatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// PrintOnScreen_Local(FString::Printf(TEXT("State: %s"), *UEnum::GetValueAsString(ComboState)));
}

void UBaseCombatComponent::InitEntityComponent(ACharacter* Character)
{
	if(bIsInitialized)
	{
		return;
	}
	
	if(!Character)
	{
		return;
	}
	
	if(ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Character))
	{
		bIsInitialized = true;

		InterruptionDuration = AttackInterruptionMontage->GetPlayLength() / AttackInterruptionMontage->RateScale;
		
		PlayerCharacter = CustomCharacter;
		if(UBaseEntityAnimationsComponent* Animations = PlayerCharacter->GetAnimation())
		{
			OnEntityRolled.AddUObject(Animations, &UBaseEntityAnimationsComponent::PlayRollMontage);
			SetRollDuration(Animations->GetRollAnimationDuration());
		}
		
		if(USprintComponent* Sprint = PlayerCharacter->GetSprintComponent())
		{
			OnEntityRolled.AddUObject(Sprint, &USprintComponent::OnRolled);
		}
		
		OnEntityRolled.AddUObject(PlayerCharacter, &ASwordslikeCharacter::OnRollStarted);

		if(PlayerCharacter->GetHealthComponent())
		{
			OnEntityRolled.AddLambda([this]()
			{
				PlayerCharacter->GetHealthComponent()->SetIsInvincible(true);
			});
			
			OnEntityRollFinished.AddLambda([this]()
			{
				PlayerCharacter->GetHealthComponent()->SetIsInvincible(false);
			});
		}
		
		OnEntityRollFinished.AddUObject(PlayerCharacter, &ASwordslikeCharacter::OnRollFinished);
		
		if(PlayerCharacter->GetWeaponHandler())
		{
			WeaponHandler = PlayerCharacter->GetWeaponHandler();
			OnForceStopAttack.AddUObject(WeaponHandler, &UWeaponHandlerComponent::OnForceStopAttack);
		}
		else
		{
			PrintOnScreen_Local(TEXT("No weapon handler"));
		}

		if(UWeaponAttackIndicatorWidget* AttackIndicator = PlayerCharacter->GetAttackIndicatorWidget())
		{
			// PrintOnScreen_Local(TEXT("UBaseCombatComponent: Set attack widget on start"), 20.f);
			AttackIndicatorWidget = AttackIndicator;
		}
		else
		{
			PrintOnScreen_Local(TEXT("UBaseCombatComponent: No widget on start"));
		}
		
		AActor* Owner = GetOwner();
		if (const ACharacter* Char = Cast<ACharacter>(Owner))
		{
			if(UAnimInstance* Anim = Char->GetMesh()->GetAnimInstance())
			{
				AnimInstance = Anim;
			}
		}

		if(CustomCharacter->IsLocallyControlled())
		{
			AnimInstance->OnMontageEnded.AddDynamic(this, &UBaseCombatComponent::OnAttackEnded);
		}
		
		bCanRoll = true;
	}
}

#pragma region Attack Action
/**
 * The input action for attacks.
 */
void UBaseCombatComponent::AttackAction()
{
	if(!WeaponHandler->HasWeapon())
	{
		PrintOnScreen_Local(TEXT("Has no weapon"));
		return;
	}
	
	if(!AnimInstance)
	{
		PrintOnScreen_Local(TEXT("Has no AnimInstance"));
		return;
	}

	if(ComboState == EComboState::Broken)
	{
		return;
	}
	
	if(ComboState == EComboState::Idle)
	{
		SetComboState(EComboState::Attacking);
		// PrintOnScreen_Local(FString::Printf(TEXT("First attack ")));
		PlayNextAnimation();
	}
	else if(ComboState == EComboState::ComboWindowOpen)
	{
		SetComboState(EComboState::ComboQueued);
	}
	else if(ComboState == EComboState::LastSecondComboWindowOpen)
	{
		SetComboState(EComboState::Attacking);
		// PrintOnScreen_Local(FString::Printf(TEXT("Late attack ")));
		PlayNextAnimation();
	}
	else
	{
		// PrintOnScreen_Local(TEXT("No case matched"));
		// PrintOnScreen_Local(FString::Printf(TEXT("State: %s"), *UEnum::GetValueAsString(ComboState)));
	}
}
#pragma endregion

#pragma region Animation
/**
 * Triggers the next combo attack animation
 */
void UBaseCombatComponent::PlayNextAnimation()
{
	CurrentAttackMontage = WeaponHandler->GetNextAttackMontage();
	PerformPlayAttackAnimation(CurrentAttackMontage);
	
	if (!HasAuthority())
	{
		Server_PlayMontage(CurrentAttackMontage);
	}
	else
	{
		Multicast_PlayMontage(CurrentAttackMontage);
	}
}

void UBaseCombatComponent::Server_PlayMontage_Implementation(UAnimMontage* Montage)
{
	Multicast_PlayMontage(Montage);
}

void UBaseCombatComponent::Multicast_PlayMontage_Implementation(UAnimMontage* Montage)
{
	if(!IsAutonomousProxy())
	{
		PerformPlayAttackAnimation(Montage);
	}
}

void UBaseCombatComponent::PerformPlayAttackAnimation(UAnimMontage* Montage)
{
	if(Montage != nullptr)
	{
		// add a 0.2f blend to the new combo animation if it's not the first attack in the combo
		if(ComboCount > 0)
		{
			FAlphaBlendArgs BlendArgs;
			BlendArgs.BlendTime = 0.2f;
			AnimInstance->Montage_StopWithBlendOut(BlendArgs, Montage);
		}

		ComboCount++;
		AnimInstance->Montage_Play(Montage);
	}
}

/**
 * Called through the Anim Notify State using the NextAttack mode
 * 
 */
void UBaseCombatComponent::PerformNextAttack()
{
	if (ComboState == EComboState::ComboQueued)
	{
		PlayNextAnimation();
		SetComboState(EComboState::Attacking);
	}
	else
	{
		SetComboState(EComboState::LastSecondComboWindowOpen);
	}
}

void UBaseCombatComponent::AllowInput()
{
	SetComboState(EComboState::ComboWindowOpen);
}

/**
 * Called when the player misses the window to continue the combo or if the current attack is the last in the combo
 * 
 */
void UBaseCombatComponent::DisableInput()
{
	SetComboState(EComboState::Ending);
}

#pragma region Attack Warnings
void UBaseCombatComponent::StartAttackWarning(const float Duration, const float AnticipationSpeedMultiplier)
{
	if(!HasAuthority())
	{
		Server_StartWarning(Duration, AnticipationSpeedMultiplier, CurrentAttackMontage);
	}
	else
	{
		AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, AnticipationSpeedMultiplier);
		Multicast_StartWarning(Duration, AnticipationSpeedMultiplier, CurrentAttackMontage);
	}
}

void UBaseCombatComponent::Server_StartWarning_Implementation(const float Duration, const float AnticipationSpeedMultiplier, UAnimMontage* Montage)
{
	AnimInstance->Montage_SetPlayRate(Montage, AnticipationSpeedMultiplier);
	Multicast_StartWarning(Duration, AnticipationSpeedMultiplier, Montage);
}

void UBaseCombatComponent::Multicast_StartWarning_Implementation(const float Duration, const float AnticipationSpeedMultiplier, UAnimMontage* Montage)
{
	AnimInstance->Montage_SetPlayRate(Montage, AnticipationSpeedMultiplier);
	
	if (!IsAutonomousProxy())
	{
		const APawn* LocalPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		if (const AActor* OwnerActor = GetOwner(); LocalPawn && OwnerActor)
		{
			const float Distance = FVector::Dist(LocalPawn->GetActorLocation(), OwnerActor->GetActorLocation());
			if (Distance <= AttackWarningRadius)
			{
				PerformStartAttackWarninig(Duration, AnticipationSpeedMultiplier, Montage);
			}
		}
		else
		{
			PrintOnScreen(FString::Printf(TEXT("Missing local or combat owner")));
		}
	}
}

void UBaseCombatComponent::PerformStartAttackWarninig(const float Duration, const float AnticipationSpeedMultiplier, UAnimMontage* Montage)
{
	if(!Montage)
	{
		return;
	}
	
	if(AttackIndicatorWidget)
	{
		AttackIndicatorWidget->Shrink(Duration / AnticipationSpeedMultiplier);
	}
	else
	{
		PrintOnScreen(TEXT("No widget"));
	}
}

void UBaseCombatComponent::EndAttackWarning()
{
	if(!HasAuthority())
	{
		Server_EndAttackWarning(CurrentAttackMontage);
	}
	else
	{
		Multicast_EndAttackWarning(CurrentAttackMontage);
	}
}

void UBaseCombatComponent::Server_EndAttackWarning_Implementation(UAnimMontage* Montage)
{
	AnimInstance->Montage_SetPlayRate(Montage, 1.0f);
	Multicast_EndAttackWarning(Montage);
}

void UBaseCombatComponent::Multicast_EndAttackWarning_Implementation(UAnimMontage* Montage)
{
	PerformEndAttackWarning(Montage);
}

void UBaseCombatComponent::PerformEndAttackWarning(UAnimMontage* Montage)
{
	AnimInstance->Montage_SetPlayRate(Montage, 1.0f);
}
#pragma endregion 

void UBaseCombatComponent::OnAttackEnded(UAnimMontage* Anim, bool bInterrupted)
{
	if(ComboState == EComboState::Idle || ComboState == EComboState::Broken)
	{
		return;
	}
	
	if(ComboState != EComboState::Ending )
	{
		return;
	}
	
	WeaponHandler->ResetAttackMontages();
	SetComboState(EComboState::Idle);
	ComboCount = 0;
}
#pragma endregion

#pragma region Force Stop Attack
/**
 * Force stops the combo or attack in place.
 * @param bIsInterruptedAttack if true, the combo state is set to broken and is only recovered after the interruption montage finishes playing.
 */
void UBaseCombatComponent::ForceStopAttack(bool bIsInterruptedAttack)
{
	if(PlayerCharacter->IsLocallyControlled())
	{
		PerformForceStopAttack(bIsInterruptedAttack);
	}
	else
	{
		Client_ForceStopAttack(bIsInterruptedAttack);
	}
}

void UBaseCombatComponent::Client_ForceStopAttack_Implementation(bool bIsInterruptedAttack)
{
	PerformForceStopAttack(bIsInterruptedAttack);
}

void UBaseCombatComponent::PerformForceStopAttack(bool bIsInterruptedAttack)
{
	ComboCount = 0;
	
	WeaponHandler->ResetAttackMontages();

	PlayInterruptionAnimation(bIsInterruptedAttack);

	if(OnForceStopAttack.IsBound())
	{
		OnForceStopAttack.Broadcast();
	}

	if(bIsInterruptedAttack)
	{
		SetComboState(EComboState::Broken);
		bIsBroken = true;

		if(GetWorld()->GetTimerManager().IsTimerActive(AttackInterruptionTimer))
		{
			GetWorld()->GetTimerManager().ClearTimer(AttackInterruptionTimer);
		}
		
		GetWorld()->GetTimerManager().SetTimer(
		AttackInterruptionTimer,
		[this]()
		{
			SetComboState(EComboState::Idle);
		},
		InterruptionDuration,
		false
		);
	}
	else
	{
		SetComboState(EComboState::Idle);
	}
	
	if (!HasAuthority())
	{
		Server_ForceStopAttack(bIsInterruptedAttack);
	}
	else
	{
		Multicast_ForceStopAttack(bIsInterruptedAttack);
	}
}

void UBaseCombatComponent::PerformForceStop(bool bIsInterrupted)
{
	
}

void UBaseCombatComponent::Server_ForceStopAttack_Implementation(bool bIsInterruptedAttack)
{
	Multicast_ForceStopAttack(bIsInterruptedAttack);
}

void UBaseCombatComponent::Multicast_ForceStopAttack_Implementation(bool bIsInterruptedAttack)
{
	PlayInterruptionAnimation(bIsInterruptedAttack);
}

void UBaseCombatComponent::PlayInterruptionAnimation(bool bIsInterruptedAttack)
{
	if(!AnimInstance)
	{
		return;
	}
	
	AnimInstance->Montage_Stop(0.1f);
	AnimInstance->Montage_Play(AttackInterruptionMontage);
}
#pragma endregion

#pragma region Roll
void UBaseCombatComponent::Roll()
{
	if(!bCanRoll || bIsRolling)
	{
		return;
	}

	bIsRolling = true;
	
	GetWorld()->GetTimerManager().SetTimer(RollTimer, this, &UBaseCombatComponent::RollRecover, RollDuration, false);
	
	if(OnEntityRolled.IsBound())
	{
		OnEntityRolled.Broadcast();
	}
}

void UBaseCombatComponent::RollRecover()
{
	bIsRolling = false;

	if(OnEntityRollFinished.IsBound())
	{
		OnEntityRollFinished.Broadcast();
	}
}

void UBaseCombatComponent::OnStunned()
{
	SetComboState(EComboState::Broken);
}

void UBaseCombatComponent::OnRecoverFromStun()
{
	SetComboState(EComboState::Idle);
}

void UBaseCombatComponent::SetComboState(const EComboState State)
{
	// if(bIsBroken)
	// {
	// 	PrintOnScreen(-1, FString::Printf(TEXT("State that overrides the break is: %s"), *UEnum::GetValueAsString(State)));
	// 	// return;
	// }
	
	ComboState = State;

	// UE_LOG(LogTemp, Log, TEXT("State: %s"), *UEnum::GetValueAsString(ComboState));
	// PrintOnScreen_Local(2, FString::Printf(TEXT("State: %s"), *UEnum::GetValueAsString(ComboState)));
}
#pragma endregion 
