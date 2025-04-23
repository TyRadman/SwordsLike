// Fill out your copyright notice in the Description page of Project Settings.


#include "ParryState_AN.h"

#include "BaseParryComponent.h"
#include "Player/SwordslikeCharacter.h"

void UParryState_AN::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                            const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if(MeshComp && MeshComp->GetOwner())
	{
		if(const ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner()))
		{
			if(!Character->IsLocallyControlled())
			{
				return;
			}
				
			Character->GetParryComponent()->SetParryState(ParryState);
		}
	}
}

FString UParryState_AN::GetNotifyName_Implementation() const
{
	return NotifyName.IsNone() ? Super::GetNotifyName_Implementation() : NotifyName.ToString();
}
