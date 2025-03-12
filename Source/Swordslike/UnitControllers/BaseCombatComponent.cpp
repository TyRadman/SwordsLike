#include "BaseCombatComponent.h"

#include "Common/WeaponHandlerComponent.h"
#include "GameFramework/Character.h"

const FName UBaseCombatComponent::ANIMATION_STOP_NOTIFY_NAME = TEXT("Stop");
const FName UBaseCombatComponent::INPUT_NOTIFY_NAME = TEXT("AllowInput");

UBaseCombatComponent::UBaseCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called when the game starts
void UBaseCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	SetIsReplicated(true);
	
	//////////////////////////////
	// cache the animInstance
	//////////////////////////////
	AActor* Owner = GetOwner();

	if (ACharacter* Char = Cast<ACharacter>(Owner))
	{
		UAnimInstance* Anim = Char->GetMesh()->GetAnimInstance();

		if(Anim)
		{
			EntityAnimInstance = Anim;
		}
	}
	
	//////////////////////////////
	// subscriptions
	//////////////////////////////
	// REVIEW: is it okay to not handle interruption? 
	EntityAnimInstance->OnMontageEnded.AddDynamic(this, &UBaseCombatComponent::OnAttackEnded);
	EntityAnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UBaseCombatComponent::OnComboNotifyBegin);
	EntityAnimInstance->OnPlayMontageNotifyEnd.AddDynamic(this, &UBaseCombatComponent::OnComboNotifyEnd);

	bCanAttack = true;
	bCanRoll = true;
}

void UBaseCombatComponent::PlayAttackAnimation()
{
	if(!EntityAnimInstance)
	{
		return;
	}
	
	if(!bIsAttacking)
	{
		bIsAttacking = true;
		EntityAnimInstance->Montage_Play(AttackAnimationMontage);
	}
	else if(bCanPerformCombo)
	{
		bIsPerformingCombo = true;
	}
}

#pragma region Force Stop Attack
void UBaseCombatComponent::ForceStopAttack()
{
	if(!EntityAnimInstance)
	{
		return;
	}

	PerformForceStop();
	
	if (GetOwnerRole() < ROLE_Authority)
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
	bIsAttacking = false;
	bIsPerformingCombo = false;
	EntityAnimInstance->Montage_Stop(0.1f);
	EntityAnimInstance->Montage_Play(AttackInterruptionMontage);
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
	bIsAttacking = false;
	bIsPerformingCombo = false;
	bCanPerformCombo = false;
}

#pragma region Animation Notify
void UBaseCombatComponent::OnComboNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if(NotifyName.IsEqual(TEXT("Damage")) && WeaponHandler)
	{
		WeaponHandler->StartWeaponAttackDetection();
	}
	
	if(NotifyName.IsEqual(INPUT_NOTIFY_NAME))
	{
		if(bIsAttacking)
		{
			bCanPerformCombo = true;
		}
	}
}

void UBaseCombatComponent::OnComboNotifyEnd(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if(NotifyName.IsEqual(TEXT("Damage")) && WeaponHandler)
	{
		WeaponHandler->StopWeaponAttackDetection();
	}
	
	if(!bIsAttacking || !bIsPerformingCombo)
	{
		EntityAnimInstance->Montage_Stop(0.5f, AttackAnimationMontage);
	}
	else
	{
		bIsPerformingCombo = false;
	}
}
#pragma endregion

#pragma region Setters
void UBaseCombatComponent::SetWeaponHandler(TObjectPtr<UWeaponHandlerComponent> Handler)
{
	if(!Handler)
	{
		UE_LOG(LogTemp, Error, TEXT("Handler pass is null in BaseCombatComponent."));
		return;
	}
	
	WeaponHandler = Handler;
}

void UBaseCombatComponent::SetCanRoll(bool CanRoll)
{
	bCanRoll = CanRoll;
}

void UBaseCombatComponent::EnableRoll()
{
	bCanRoll = true;
}

void UBaseCombatComponent::DisableRoll()
{
	bCanRoll = false;
}
#pragma endregion

#pragma region Network
void UBaseCombatComponent::AttackAction()
{
	if(!bCanAttack)
	{
		return;
	}
	
	PlayAttackAnimation();
	
	if (GetOwnerRole() < ROLE_Authority)
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
		PlayAttackAnimation();
	}
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

void UBaseCombatComponent::SetRollDuration(float Duration)
{
	RollDuration = Duration;
}
#pragma endregion 
