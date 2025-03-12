// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHealthBar.h"

#include "Components/ProgressBar.h"

void UPlayerHealthBar::SetHealthBarValue(float CurrentHealth, float MaxHealth)
{
	if(HealthBar)
	{
		float amount = CurrentHealth / MaxHealth;
		HealthBar->SetPercent(amount);
	}
}

void UPlayerHealthBar::SetStaminaBarValue(float CurrentHealth, float MaxHealth)
{
	if(StaminaBar)
	{
		float amount = CurrentHealth / MaxHealth;
		StaminaBar->SetPercent(amount);
	}
}

void UPlayerHealthBar::NativeConstruct()
{
	Super::NativeConstruct();
	
}

