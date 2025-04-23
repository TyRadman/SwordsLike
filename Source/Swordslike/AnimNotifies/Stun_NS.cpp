#include "Stun_NS.h"

#include "BaseParryComponent.h"
#include "Player/SwordslikeCharacter.h"

void UStun_NS::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	if(MeshComp && MeshComp->GetOwner())
	{
		if(ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner()))
		{
			if(!Character->IsLocallyControlled())
			{
				return;
			}
			
			Character->OnStunned();
			
			if(UBaseParryComponent* Parry = Character->GetParryComponent())
			{
				// Parry->OnStunned();
			}
		}
	}
}

void UStun_NS::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if(MeshComp && MeshComp->GetOwner())
	{
		if(ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner()))
		{
			if(!Character->IsLocallyControlled())
			{
				return;
			}
			
			Character->OnStunnedRecover();
			
			if(UBaseParryComponent* Parry = Character->GetParryComponent())
			{
				Parry->OnRecoverFromStun();
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Red, TEXT("No Parry Component"));
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Red, TEXT("No ASwordslikeCharacter"));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Red, TEXT("No MeshComp"));
	}
}
