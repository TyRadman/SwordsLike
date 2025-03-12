#include "PlayerHealthBar.h"

#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"

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

void UPlayerHealthBar::SetPostureBarValue(float Current, float Max)
{
	if(PostureBar)
	{
		float amount = Current / Max;
		PostureBar->SetPercent(amount);

		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
		// 	FString::Printf(TEXT("Posture values: %f / %f"), Current, Max));
	}
}

void UPlayerHealthBar::NativeConstruct()
{
	Super::NativeConstruct();
}

