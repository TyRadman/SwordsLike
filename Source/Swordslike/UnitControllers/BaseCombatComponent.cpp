#include "BaseCombatComponent.h"

#include "BaseEntityAnimationsComponent.h"
#include "Common/WeaponHandlerComponent.h"
#include "GameFramework/Character.h"
#include "Player/SwordslikeCharacter.h"

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
		
		SetWeaponHandler(PlayerCharacter->GetWeaponHandler());
		
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
		PrintOnScreen_Local(TEXT("Started attacking"));
		bIsAttacking = true;
		PlayNextAnimation();
	}
	else if(bCanPerformCombo)
	{
		PrintOnScreen_Local(TEXT("Combo"));
		bIsPerformingCombo = true;

		if(bIdealNextAttackPointPassed)
		{
			PrintOnScreen_Local(TEXT("Late Combo"));
			bIdealNextAttackPointPassed = false;
			PlayNextAnimation();
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
	if(UAnimMontage* AttackMontage = WeaponHandler->GetNextAttackMontage(); AttackMontage != nullptr)
	{
		if(ComboCount > 0)
		{
			bIsEndOfCombo = false;
			FAlphaBlendArgs BlendArgs;
			BlendArgs.BlendTime = 0.2f;
			AnimInstance->Montage_StopWithBlendOut(BlendArgs, AttackMontage);
		}

		ComboCount++;
		AnimInstance->Montage_Play(AttackMontage);
	}
}

void UBaseCombatComponent::AllowInput()
{
	if(bIsAttacking)
	{
		bCanPerformCombo = true;
	}
}

void UBaseCombatComponent::PerformNextAttack()
{
	bIdealNextAttackPointPassed = true;
	
	if(bIsPerformingCombo)
	{
		PrintOnScreen_Local(TEXT("Combo 2"));
		bIdealNextAttackPointPassed = false;
		PlayNextAnimation();
	}
	else
	{
		PrintOnScreen_Local(TEXT("Combo 2 failed"));
	}
}

void UBaseCombatComponent::DisableInput()
{
	bIsPerformingCombo = false;
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
		PrintOnScreen_Local(TEXT("OnAttackEnded Prevented"), FColor::Purple);
		return;
	}
	
	PrintOnScreen_Local(TEXT("OnAttackEnded"), FColor::Purple);
	WeaponHandler->ResetAttackMontages();
	bIdealNextAttackPointPassed = false;
	bIsAttacking = false;
	bCanPerformCombo = false;
	bIsPerformingCombo = false;
	ComboCount = 0;
}

#pragma region Setters
void UBaseCombatComponent::SetWeaponHandler(const TObjectPtr<UWeaponHandlerComponent>& Handler)
{
	if(!Handler)
	{
		UE_LOG(LogTemp, Error, TEXT("Handler pass is null in BaseCombatComponent."));
		return;
	}
	
	WeaponHandler = Handler;
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
