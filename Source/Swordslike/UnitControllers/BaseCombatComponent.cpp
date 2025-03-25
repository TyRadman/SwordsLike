#include "BaseCombatComponent.h"

#include "BaseEntityAnimationsComponent.h"
#include "Common/WeaponHandlerComponent.h"
#include "GameFramework/Character.h"
#include "Player/SwordslikeCharacter.h"

UBaseCombatComponent::UBaseCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBaseCombatComponent::InitEntityComponent(ACharacter* Character)
{
	if(Character && Cast<ASwordslikeCharacter>(Character))
	{
		PlayerCharacter = Cast<ASwordslikeCharacter>(Character);
		
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
	}
}

void UBaseCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	SetIsReplicated(true);
	
	AActor* Owner = GetOwner();

	if (ACharacter* Char = Cast<ACharacter>(Owner))
	{
		UAnimInstance* Anim = Char->GetMesh()->GetAnimInstance();

		if(Anim)
		{
			AnimInstance = Anim;
		}
	}
	
	AnimInstance->OnMontageEnded.AddDynamic(this, &UBaseCombatComponent::OnAttackEnded);

	bCanAttack = true;
	bCanRoll = true;
}

#pragma region Attack Action
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

#pragma region Animation
void UBaseCombatComponent::PlayAttackAnimation()
{
	if(!AnimInstance)
	{
		return;
	}
	
	if(!bIsAttacking)
	{
		bIsAttacking = true;
		AnimInstance->Montage_Play(WeaponHandler->GetAttackMontage());
	}
	else if(bCanPerformCombo)
	{
		bIsPerformingCombo = true;
	}
}

void UBaseCombatComponent::AllowInput()
{
	if(bIsAttacking)
	{
		bCanPerformCombo = true;
	}
}

void UBaseCombatComponent::DisableInput()
{
	if(!bIsAttacking || !bIsPerformingCombo)
	{
		AnimInstance->Montage_Stop(0.5f, WeaponHandler->GetAttackMontage());
	}
	else
	{
		bIsPerformingCombo = false;
	}
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
	AnimInstance->Montage_Stop(0.1f);
	AnimInstance->Montage_Play(AttackInterruptionMontage);
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
