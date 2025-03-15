// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HUDManager.generated.h"

class UMasterHUD;
class UPlayerHealthBar;
/**
 * The HUD that holds the stats of the player such as HP, Stamina, and Posture.
 */
UCLASS()
class SWORDSLIKE_API AHUDManager : public AHUD
{
	GENERATED_BODY()

public:
	AHUDManager();

	FORCEINLINE UMasterHUD* GetMasterHUD() const { return MasterHUD; }

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UMasterHUD* MasterHUD;

	void CreateHealthBar();
};
