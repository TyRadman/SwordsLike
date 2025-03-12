// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HUDManager.generated.h"

class UPlayerHealthBar;
/**
 * 
 */
UCLASS()
class SWORDSLIKE_API AHUDManager : public AHUD
{
	GENERATED_BODY()

public:
	AHUDManager();

	/**
	 * Binds the character's health amount to the HUD health bar.
	 * @param Character The character controller that has a BaseHealthComponent or a derived class from it.
	 */
	void BindHealthBar(ACharacter* Character);
	void BindStaminaBar(ACharacter* Character);
	void BindPostureBar(ACharacter* Character);

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UPlayerHealthBar* PlayerStats;

	void CreateHealthBar();
};
