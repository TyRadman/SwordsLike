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
	const int32 HitTypeIndex = static_cast<int32>(DamageInfo.HitType);
	
	if (ForwardDot > 0.7f)
	{
		HitReactionMontage = ForwardHitReactMontages[HitTypeIndex];
	}
	else if (ForwardDot < -0.7f)
	{
		HitReactionMontage = BackwardHitReactMontages[HitTypeIndex];
	}
	else if (RightDot > 0.f)
	{
		HitReactionMontage = RightHitReactMontages[HitTypeIndex];
	}
	else
	{
		HitReactionMontage = LeftHitReactMontages[HitTypeIndex];
	}

	
	// PrintOnScreen_Local(FString::Printf(TEXT("Montage for hit is: (%d) %s"), HitTypeIndex, *HitReactionMontage->GetName()));
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
void UBaseEntityAnimationsComponent::PlayMontage(UAnimMontage* Montage, const bool bForcePlay)
{
	// PrintOnScreen(FString::Printf(TEXT("Called %s to play"), *Montage->GetName()));
	PerformPlayMontage(Montage, bForcePlay);
	
	if (!HasAuthority())
	{
		Server_PlayRollMontage(Montage, bForcePlay);
	}
	else
	{
		Multicast_PlayRollMontage(Montage, bForcePlay);
	}
}

void UBaseEntityAnimationsComponent::Server_PlayRollMontage_Implementation(UAnimMontage* Montage, const bool bForcePlay)
{
	Multicast_PlayRollMontage(Montage, bForcePlay);
}

void UBaseEntityAnimationsComponent::Multicast_PlayRollMontage_Implementation(UAnimMontage* Montage, const bool bForcePlay)
{
	if(IsLocallyControlled())
	{
		return;
	}
	
	PerformPlayMontage(Montage, bForcePlay);
}

void UBaseEntityAnimationsComponent::PerformPlayMontage(UAnimMontage* Montage, const bool bForcePlay)
{
	if(bForcePlay)
	{
		AnimInstance->Montage_Stop(0.1f);
	}
	
	AnimInstance->Montage_Play(Montage);
}
#pragma endregion