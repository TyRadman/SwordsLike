#include "BaseCombatComponent.h"

#include "BaseEntityAnimationsComponent.h"
#include "Common/WeaponHandlerComponent.h"
#include "GameFramework/Character.h"
#include "Player/SwordslikeCharacter.h"
#include "Swordslike/UI/WorldUIElements/WeaponAttackIndicatorWidget.h"

UBaseCombatComponent::UBaseCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
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
		bCanAttack = true;
		bCanRoll = true;
	}
}

#pragma region Attack Action
/**
 * The input action for attacks.
 */
void UBaseCombatComponent::AttackAction()
{
	if(!bCanAttack)
	{
		PrintOnScreen_Local(TEXT("Can't attack"));
		return;
	}
	
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
	
	if(!bIsAttacking && !AnimInstance->Montage_IsPlaying(WeaponHandler->GetAttackMontage()))
	{
		bIsAttacking = true;
		PlayNextAnimation();
	}
	else if(bCanPerformCombo)
	{
		bIsPerformingCombo = true;

		if(bIdealNextAttackPointPassed)
		{
			// PrintOnScreen_Local(TEXT("Late combo"));
			bIdealNextAttackPointPassed = false;
			PlayNextAnimation();
		}
		else
		{
			// PrintOnScreen_Local(TEXT("Early Combo"));
		}
	}
	else
	{
		PrintOnScreen_Local(TEXT("Something went wrong"));
	}
}
#pragma endregion

#pragma region Animation
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
		if(ComboCount > 0)
		{
			bIsEndOfCombo = false;
			FAlphaBlendArgs BlendArgs;
			BlendArgs.BlendTime = 0.2f;
			AnimInstance->Montage_StopWithBlendOut(BlendArgs, CurrentAttackMontage);
		}

		ComboCount++;
		AnimInstance->Montage_Play(CurrentAttackMontage);
	}
}

void UBaseCombatComponent::AllowInput()
{
	bCanPerformCombo = true;
}

void UBaseCombatComponent::PerformNextAttack()
{
	bIdealNextAttackPointPassed = true;
	
	if(bIsPerformingCombo)
	{
		// PrintOnScreen_Local(TEXT("Auto perform combo"));
		bIdealNextAttackPointPassed = false;
		PlayNextAnimation();
	}
	else
	{
		// PrintOnScreen_Local(TEXT("Not auto perform combo"));
	}
}

void UBaseCombatComponent::DisableInput()
{
	bIsPerformingCombo = false;
}

void UBaseCombatComponent::StartAttackWarning(const float Duration)
{
	if(!HasAuthority())
	{
		Server_StartWarning(Duration);
	}
	else
	{
		AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, AnticipationMultiplier);
		Multicast_StartWarning(Duration);
	}
}

void UBaseCombatComponent::Server_StartWarning_Implementation(const float Duration)
{
	AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, AnticipationMultiplier);
	Multicast_StartWarning(Duration);
}

void UBaseCombatComponent::Multicast_StartWarning_Implementation(const float Duration)
{
	AnimInstance->Montage_SetPlayRate(CurrentAttackMontage, AnticipationMultiplier);
	
	if (!IsAutonomousProxy())
	{
		const APawn* LocalPawn = GetWorld()->GetFirstPlayerController()->GetPawn();
		if (const AActor* OwnerActor = GetOwner(); LocalPawn && OwnerActor)
		{
			const float Distance = FVector::Dist(LocalPawn->GetActorLocation(), OwnerActor->GetActorLocation());
			// PrintOnScreen(FString::Printf(TEXT("Distance: %f"), Distance));
			if (Distance <= AttackWarningRadius)
			{
				PerformStartAttackWarninig(Duration);
			}
		}
		else
		{
			PrintOnScreen(FString::Printf(TEXT("Missing local or combat owner")));
		}
	}
}

void UBaseCombatComponent::PerformStartAttackWarninig(const float Duration)
{
	if(AttackIndicatorWidget && CurrentAttackMontage)
	{
		AttackIndicatorWidget->Shrink(Duration / AnticipationMultiplier);
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

void UBaseCombatComponent::PerformForceStop()
{
	PrintOnScreen_Local(TEXT("ForceStop"), FColor::Blue);
	bIsAttacking = false;
	bIsPerformingCombo = false;
	AnimInstance->Montage_Stop(0.1f);
	AnimInstance->Montage_Play(AttackInterruptionMontage);
	ComboCount = 0;
}

void UBaseCombatComponent::Server_ForceStopAttack_Implementation()
{
	Multicast_ForceStopAttack();
}

void UBaseCombatComponent::Multicast_ForceStopAttack_Implementation()
{
	PerformForceStop();
}
#pragma endregion

void UBaseCombatComponent::OnAttackEnded(UAnimMontage* Anim, bool bInterrupted)
{
	if(!bIsEndOfCombo)
	{
		bIsEndOfCombo = true;
		// PrintOnScreen_Local(TEXT("OnAttackEnded Prevented"), FColor::Purple);
		return;
	}
	
	// PrintOnScreen_Local(TEXT("OnAttackEnded"), FColor::Purple);
	WeaponHandler->ResetAttackMontages();
	bIdealNextAttackPointPassed = false;
	bIsAttacking = false;
	bCanPerformCombo = false;
	bIsPerformingCombo = false;
	ComboCount = 0;
}

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
