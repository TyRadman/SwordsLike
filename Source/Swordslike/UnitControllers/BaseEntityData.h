// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "BaseEntityData.generated.h"

/**
 * Holds the basic stats of an entity in the game.
 */
UCLASS()
class SWORDSLIKE_API UBaseEntityData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mobility")
	float MovementSpeed = 500.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mobility")
	float SprintSpeed = 700.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Mobility")
	float JumpHeight = 700.0f;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float MaxHealthPoints = 200.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats")
	float MaxStamina = 100.f;
};
