#include "BaseCombatComponent.h"

#include "BaseEntityAnimationsComponent.h"
#include "BaseParryComponent.h"
#include "Common/WeaponHandlerComponent.h"
#include "GameFramework/Character.h"
#include "Player/SwordslikeCharacter.h"
#include "Swordslike/UI/WorldUIElements/WeaponAttackIndicatorWidget.h"

UBaseCombatComponent::UBaseCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UBaseCombatComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// PlayerCharacter->PrintOverhead(*UEnum::GetValueAsString(ComboState));
}

void UBaseCombatComponent::InitEntityComponent(ACharacter* Character)
{
	if(!Character)
	{
		return;
	}
	
	if(ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Character))
	{
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
		}
		else
		{
			PrintOnScreen_Local(TEXT("No weapon handler"));
		}

		if(UWeaponAttackIndicatorWidget* AttackIndicator = PlayerCharacter->GetAttackIndicatorWidget())
		{
			AttackIndicatorWidget = AttackIndicator;
		}
		else
		{
			PrintOnScreen_Local(TEXT("No widget on start"));
		}
		
		AActor* Owner = GetOwner();
		if (const ACharacter* Char = Cast<ACharacter>(Owner))
		{
			if(UAnimInstance* Anim = Char->GetMesh()->GetAnimInstance())
			{
				AnimInstance = Anim;
			}
		}
	
		AnimInstance->OnMontageEnded.AddDynamic(this, &UBaseCombatComponent::OnAttackEnded);
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
	
	if(ComboState == EComboState::Idle)
	{
		ComboState = EComboState::Attacking;
		PrintOnScreen_Local(FString::Printf(TEXT("First attack ")));
		PlayNextAnimation();
	}
	else if(ComboState == EComboState::ComboWindowOpen)
	{
		ComboState = EComboState::ComboQueued;
	}
	else if(ComboState == EComboState::LastSecondComboWindowOpen)
	{
		ComboState = EComboState::Attacking;
		PrintOnScreen_Local(FString::Printf(TEXT("Late attack ")));
		PlayNextAnimation();
	}
}
#pragma endregion

#pragma region Animation
/**
 * Triggers the next combo attack animation
 */
void UBaseCombatComponent::PlayNextAnimation()
{
	PerformPlayAttackAnimation();
	
	if (!HasAuthority())
	{
		Server_PlayMontage();
	}
	else
	{
		Multicast_PlayMontage();
	}
}

void UBaseCombatComponent::Server_PlayMontage_Implementation()
{
	Multicast_PlayMontage();
}

void UBaseCombatComponent::Multicast_PlayMontage_Implementation()
{
	if(!IsAutonomousProxy())
	{
		PerformPlayAttackAnimation();
	}
}

void UBaseCombatComponent::PerformPlayAttackAnimation()
{
	CurrentAttackMontage = WeaponHandler->GetNextAttackMontage();
	if(CurrentAttackMontage != nullptr)
	{
		// add a 0.2f blend to the new combo animation if it's not the first attack in the combo
		if(ComboCount > 0)
		{
			FAlphaBlendArgs BlendArgs;
			BlendArgs.BlendTime = 0.2f;
			AnimInstance->Montage_StopWithBlendOut(BlendArgs, CurrentAttackMontage);
		}

		ComboCount++;
		AnimInstance->Montage_Play(CurrentAttackMontage);
	}
}

/** Called through the Anim Notify State using the NextAttack mode */
void UBaseCombatComponent::PerformNextAttack()
{
	if (ComboState == EComboState::ComboQueued)
	{
		PlayNextAnimation();
		ComboState = EComboState::Attacking;
	}
	else
	{
		ComboState = EComboState::LastSecondComboWindowOpen;
	}
}

void UBaseCombatComponent::AllowInput()
{
	ComboState = EComboState::ComboWindowOpen;
}

/** Called when the player misses the window to continue the combo or if the current attack is the last in the combo */
void UBaseCombatComponent::DisableInput()
{
	ComboState = EComboState::Ending;
}

#pragma region Attack Warnings
void UBaseCombatComponent::StartAttackWarning(const float Duration, const float AnticipationSpeedMultiplier)
{
	if(!HasAuthority())
	{
		Server_StartWarning(Duration, AnticipationSpeedMultiplier);
	}
	else
	{
		AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, AnticipationSpeedMultiplier);
		Multicast_StartWarning(Duration, AnticipationSpeedMultiplier);
	}
}

void UBaseCombatComponent::Server_StartWarning_Implementation(const float Duration, const float AnticipationSpeedMultiplier)
{
	AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, AnticipationSpeedMultiplier);
	Multicast_StartWarning(Duration, AnticipationSpeedMultiplier);
}

void UBaseCombatComponent::Multicast_StartWarning_Implementation(const float Duration, const float AnticipationSpeedMultiplier)
{
	AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, AnticipationSpeedMultiplier);
	
	if (!IsAutonomousProxy())
	{
		const APawn* LocalPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		if (const AActor* OwnerActor = GetOwner(); LocalPawn && OwnerActor)
		{
			const float Distance = FVector::Dist(LocalPawn->GetActorLocation(), OwnerActor->GetActorLocation());
			// PrintOnScreen(FString::Printf(TEXT("Distance: %f"), Distance));
			if (Distance <= AttackWarningRadius)
			{
				PerformStartAttackWarninig(Duration, AnticipationSpeedMultiplier);
			}
		}
		else
		{
			PrintOnScreen(FString::Printf(TEXT("Missing local or combat owner")));
		}
	}
}

void UBaseCombatComponent::PerformStartAttackWarninig(const float Duration, const float AnticipationSpeedMultiplier)
{
	if(AttackIndicatorWidget && CurrentAttackMontage)
	{
		AttackIndicatorWidget->Shrink(Duration / AnticipationSpeedMultiplier);
	}
	else
	{
		PrintOnScreen_Local(TEXT("No widget"));
	}
}

void UBaseCombatComponent::EndAttackWarning()
{
	if(!HasAuthority())
	{
		Server_EndAttackWarning();
	}
	else
	{
		Multicast_EndAttackWarning();
	}
}

void UBaseCombatComponent::Server_EndAttackWarning_Implementation()
{
	AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, 1.0f);
	Multicast_EndAttackWarning();
}

void UBaseCombatComponent::Multicast_EndAttackWarning_Implementation()
{
	PerformEndAttackWarning();
}

void UBaseCombatComponent::PerformEndAttackWarning()
{
	AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, 1.0f);
}
#pragma endregion 

void UBaseCombatComponent::OnAttackEnded(UAnimMontage* Anim, bool bInterrupted)
{
	if(ComboState == EComboState::Idle)
	{
		return;
	}
	
	if(ComboState != EComboState::Ending)
	{
		PrintOnScreen_Local(TEXT("OnAttackEnded Prevented"), FColor::Purple);
		return;
	}
	
	PrintOnScreen_Local(TEXT("OnAttackEnded"), FColor::Purple);
	WeaponHandler->ResetAttackMontages();
	ComboState = EComboState::Idle;
	ComboCount = 0;
}
#pragma endregion

#pragma region Force Stop Attack
void UBaseCombatComponent::ForceStopAttack()
{
	if(!AnimInstance)
	{
		return;
	}

	PerformForceStop();
	
	if (!HasAuthority())
	{
		Server_ForceStopAttack();
	}
	else
	{
		Multicast_ForceStopAttack();
	}
}

void UBaseCombatComponent::Server_ForceStopAttack_Implementation()
{
	Multicast_ForceStopAttack();
}

void UBaseCombatComponent::Multicast_ForceStopAttack_Implementation()
{
	PerformForceStop();
}

void UBaseCombatComponent::PerformForceStop()
{
	ComboState = EComboState::Idle;
	AnimInstance->Montage_Stop(0.1f);
	AnimInstance->Montage_Play(AttackInterruptionMontage);
	ComboCount = 0;
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
#pragma endregion 
