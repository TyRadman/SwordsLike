// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHealthBar.generated.h"

class UProgressBar;

/**
 * A UserWidget that displays the player's HP and Stamina (Name should be changed.) 
 */
UCLASS()
class SWORDSLIKE_API UPlayerHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	// So that we can bind this to the Progress Bar in your widget
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UProgressBar* StaminaBar;

	void SetHealthBarValue(float CurrentHealth, float MaxHealth);
	void SetStaminaBarValue(float CurrentStamina, float MaxStamina);

protected:
	// Override the Initialize function to set up the widget
	virtual  void NativeConstruct() override;
};
