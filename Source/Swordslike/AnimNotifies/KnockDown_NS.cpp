#include "KnockDown_NS.h"

#include "BaseParryComponent.h"
#include "Player/SwordslikeCharacter.h"

void UKnockDown_NS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if(MeshComp && MeshComp->GetOwner())
	{
		if(ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner()))
		{
			Character->SetCanMove(false);
			Character->SetCanJump(false);
			Character->SetCanAttack(false);
			
			if(UBaseParryComponent* Parry = Character->GetParryComponent())
			{
				Parry->StartRecoveryFromKnockDown();
			}

			if(UPlayerHealthComponent* Health = Character->GetHealthComponent())
			{
				Health->SetIsInvincible(true);
			}
		}
	}
}

void UKnockDown_NS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if(MeshComp)
	{
		if(ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner()))
		{
			Character->SetCanMove(true);
			Character->SetCanJump(true);
			Character->SetCanAttack(true);
			
			if(UBaseParryComponent* Parry = Character->GetParryComponent())
			{
				Parry->EndRecoveryFromKnockDown();
			}

			if(UPlayerHealthComponent* Health = Character->GetHealthComponent())
			{
				Health->SetIsInvincible(false);
			}
		}
	}
}
