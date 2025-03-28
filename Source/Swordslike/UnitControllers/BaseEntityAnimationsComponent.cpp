#include "BaseEntityAnimationsComponent.h"
#include "GameFramework/Character.h"
#include "Player/SwordslikeCharacter.h"

UBaseEntityAnimationsComponent::UBaseEntityAnimationsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UBaseEntityAnimationsComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	
}

void UBaseEntityAnimationsComponent::InitEntityComponent(ACharacter* Character)
{
	if(Character)
	{
		if(ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Character))
		{
			OwnerCharacter = CustomCharacter;
		}

		if(const ACharacter* Character1 = Cast<ACharacter>(GetOwner()))
		{
			AnimInstance = Character1->GetMesh()->GetAnimInstance();
		}
	}
}

#pragma region Hit React Animation
void UBaseEntityAnimationsComponent::PlayHitReactMontage(const FDamageInfo& DamageInfo)
{
	const FVector Forward = OwnerCharacter->GetActorForwardVector();
	const FVector Right = OwnerCharacter->GetActorRightVector();
	const FVector AttackLocation = DamageInfo.DamageInstigator->GetActorLocation();
	const FVector HitDirection = (AttackLocation - OwnerCharacter->GetActorLocation()).GetSafeNormal();

	const float ForwardDot = FVector::DotProduct(Forward, HitDirection);
	const float RightDot   = FVector::DotProduct(Right, HitDirection);

	UAnimMontage* HitReactionMontage;
	
	if (ForwardDot > 0.7f)
	{
		HitReactionMontage = ForwardHitReactMontage;
	}
	else if (ForwardDot < -0.7f)
	{
		HitReactionMontage = BackwardHitReactMontage;
	}
	else if (RightDot > 0.f)
	{
		HitReactionMontage = RightHitReactMontage;
	}
	else
	{
		HitReactionMontage = LeftHitReactMontage;
	}
	
	PlayMontage(HitReactionMontage);
}
#pragma endregion 

#pragma region Roll Animation
UAnimMontage* UBaseEntityAnimationsComponent::GetRollMontage() const
{
	if(!OwnerCharacter)
	{
		return RollMontage_F;
	}

	const FVector2D MovementVector = OwnerCharacter->GetMovementVector();
	const bool IsLocked = OwnerCharacter->GetLockOnComponent()->GetIsLocked();
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
	PlayMontage(GetRollMontage());
}
#pragma endregion 

#pragma region Utilies
void UBaseEntityAnimationsComponent::PlayMontage(UAnimMontage* Montage)
{
	PerformPlayMontage(Montage);
	
	if(!HasAuthority())
	{
		Server_PlayRollMontage(Montage);
	}
	else
	{
		Multicast_PlayRollMontage(Montage);
	}
}

void UBaseEntityAnimationsComponent::Server_PlayRollMontage_Implementation(UAnimMontage* Montage)
{
	Multicast_PlayRollMontage(Montage);
}

void UBaseEntityAnimationsComponent::Multicast_PlayRollMontage_Implementation(UAnimMontage* Montage)
{
	PerformPlayMontage(Montage);
}

void UBaseEntityAnimationsComponent::PerformPlayMontage(UAnimMontage* Montage)
{
	AnimInstance->Montage_Play(Montage);
}
#pragma endregion