#include "DodgeRoll_NS.h"

#include "Player/SwordslikeCharacter.h"

void UDodgeRoll_NS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if(MeshComp->GetOwner())
	{
		if(ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner()))
		{
			if(Character->IsLocallyControlled())
			{
				UE_LOG(LogTemp, Warning, TEXT("DodgeRoll_NS::NotifyBegin"));
				Character->EnableDestructibleCollider();
			}
		}
	}
}

void UDodgeRoll_NS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	
	if(MeshComp->GetOwner())
	{
		if(ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner()))
		{
			if(Character->IsLocallyControlled())
			{
				UE_LOG(LogTemp, Warning, TEXT("DodgeRoll_NS::NotifyEnd"));
				Character->DisableDestructibleCollider();
			}
		}
	}
}
