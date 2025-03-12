// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * Displays an overhead health bar.
 */
UCLASS()
class SWORDSLIKE_API UOverheadHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* PostureBar;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameTextBlock;
	
	void SetHealthBarValue(float CurrentHealth, float MaxHealth);
	void SetPostureBarValue(float Current, float Max);
	void SetNameValue(FText NameText);

	void Show();
	void Hide();
	
protected:
	// Override the Initialize function to set up the widget
	virtual void NativeConstruct() override;
};
