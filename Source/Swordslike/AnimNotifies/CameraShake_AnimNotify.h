// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "CameraShake_AnimNotify.generated.h"

class UCameraShakeBase;

UCLASS()
class SWORDSLIKE_API UCameraShake_AnimNotify : public UAnimNotify
{
	GENERATED_BODY()

	TSubclassOf<UCameraShakeBase> CameraShakeClass;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
