// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

/**
 * 
 */
UINTERFACE(Blueprintable)
class SWORDSLIKE_API UInteractable : public UInterface
{
  GENERATED_BODY()
};

class SWORDSLIKE_API IInteractable
{
  GENERATED_BODY()

public:
	virtual void Interact(AActor* Interactor) = 0;
};
