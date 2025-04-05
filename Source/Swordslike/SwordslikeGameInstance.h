// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SwordslikeGameInstance.generated.h"

class UPlayerStartCharacterDataAsset;
/**
 * 
 */
UCLASS()
class SWORDSLIKE_API USwordslikeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=CharactersData)
	TArray<TSoftObjectPtr<UPlayerStartCharacterDataAsset>> PlayerCharactersData;
};
