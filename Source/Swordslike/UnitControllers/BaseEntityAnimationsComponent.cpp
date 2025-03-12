// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseEntityAnimationsComponent.h"
#include "GameFramework/Character.h"
#include "Player/SwordslikeCharacter.h"

// Sets default values for this component's properties
UBaseEntityAnimationsComponent::UBaseEntityAnimationsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBaseEntityAnimationsComponent::InitEntityComponent(ACharacter* Character)
{
	if(Character)
	{
		if(ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Character))
		{
			OwnerCharacter = CustomCharacter;
		}
	}
}

void UBaseEntityAnimationsComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

float UBaseEntityAnimationsComponent::GetRollAnimationDuration() const
{
	// PrintOnScreen(FString::Printf(TEXT("Length of a roll: %f"), RollMontage_F->GetPlayLength()));
	return RollMontage_F->GetPlayLength();
}


// Called when the game starts
void UBaseEntityAnimationsComponent::BeginPlay()
{
	Super::BeginPlay();

	if(ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		AnimInstance = Character->GetMesh()->GetAnimInstance();
	}
}

#pragma region Hit React Animation
void UBaseEntityAnimationsComponent::PlayHitReactMontage()
{
	PlayMontage(HitReactMontage);
}
#pragma endregion 

#pragma region Roll Animation
UAnimMontage* UBaseEntityAnimationsComponent::GetRollMontage()
{
	if(!OwnerCharacter)
	{
		return RollMontage_F;
	}
	
	FVector2D MovementVector = OwnerCharacter->GetMovementVector();
	bool IsLocked = OwnerCharacter->GetLockOnComponent()->bIsLockedOnTarget;

	if(!IsLocked)
	{
		return RollMontage_F;
	}

	if(FMath::Abs(MovementVector.X) > FMath::Abs(MovementVector.Y))
	{
		return MovementVector.X > 0 ? RollMontage_R : RollMontage_L;
	}
	else
	{
		return MovementVector.Y > 0 ? RollMontage_F : RollMontage_B;
	}
}

void UBaseEntityAnimationsComponent::PlayRollMontage()
{
	UAnimMontage* Anim = GetRollMontage();
	PlayMontage(Anim);

	if(GetOwnerRole() < ROLE_Authority)
	{
		Server_PlayRollMontage(Anim);
	}
	else
	{
		Multicast_PlayRollMontage(Anim);
	}
}

void UBaseEntityAnimationsComponent::Server_PlayRollMontage_Implementation(UAnimMontage* Montage)
{
	Multicast_PlayRollMontage(Montage);
}

void UBaseEntityAnimationsComponent::Multicast_PlayRollMontage_Implementation(UAnimMontage* Montage)
{
	PlayMontage(Montage);
}
#pragma endregion 

#pragma region Utilies
void UBaseEntityAnimationsComponent::PlayMontage(UAnimMontage* Montage)
{
	AnimInstance->Montage_Play(Montage);
}
#pragma endregion