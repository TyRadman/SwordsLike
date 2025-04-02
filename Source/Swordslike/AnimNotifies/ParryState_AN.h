// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ParryState_AN.generated.h"

enum class EParryState : uint8;
/**
 * 
 */
UCLASS()
class SWORDSLIKE_API UParryState_AN : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EParryState ParryState;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName NotifyName;
	virtual FString GetNotifyName_Implementation() const override;
};
