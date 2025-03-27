#include "PlayerHealthBar.h"

#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "BaseParryComponent.h"
#include "SprintComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Player/SwordslikeCharacter.h"
#include "Player/PlayerHealthComponent.h"


void UPlayerHealthBar::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPlayerHealthBar::SetHealthBarSize(const float MaxHealth)
{
	if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(HealthBar->Slot))
	{
		PanelSlot->SetSize(FVector2D(MaxHealth * PixelsPerHealthPoint, PanelSlot->GetSize().Y));
	}
}

void UPlayerHealthBar::SetStaminaBarSize(const float MaxStamina)
{
	if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(StaminaBar->Slot))
	{
		PanelSlot->SetSize(FVector2D(MaxStamina * PixelsPerStaminaPoint, PanelSlot->GetSize().Y));
	}
}

void UPlayerHealthBar::SetHealthBarValue(const float CurrentHealth, const float MaxHealth)
{
	if(HealthBar)
	{
		const float Amount = CurrentHealth / MaxHealth;
		HealthBar->SetPercent(Amount);
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%f / %f"), CurrentHealth, MaxHealth));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "HealthBar is NULL");
	}
}

void UPlayerHealthBar::SetStaminaBarValue(const float CurrentStamina, const float MaxStamina)
{
	if(StaminaBar)
	{
		float amount = CurrentStamina / MaxStamina;
		StaminaBar->SetPercent(amount);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "StaminaBar is NULL");
	}
}

void UPlayerHealthBar::SetPostureBarValue(const float Current, const float Max)
{
	if(PostureBar)
	{
		float amount = Current / Max;
		PostureBar->SetPercent(amount);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "PostureBar is NULL");
	}
}