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

void UPlayerHealthBar::SetHealthBarSize(float MaxHealth)
{
	if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(HealthBar->Slot))
	{
		PanelSlot->SetSize(FVector2D(MaxHealth * PixelsPerHealthPoint, PanelSlot->GetSize().Y));
	}
}

void UPlayerHealthBar::SetStaminaBarSize(float MaxStamina)
{
	if (UCanvasPanelSlot* PanelSlot = Cast<UCanvasPanelSlot>(StaminaBar->Slot))
	{
		PanelSlot->SetSize(FVector2D(MaxStamina * PixelsPerStaminaPoint, PanelSlot->GetSize().Y));
	}
}

void UPlayerHealthBar::SetHealthBarValue(float CurrentHealth, float MaxHealth)
{
	if(HealthBar)
	{
		float amount = CurrentHealth / MaxHealth;
		HealthBar->SetPercent(amount);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "HealthBar is NULL");
	}
}

void UPlayerHealthBar::SetStaminaBarValue(float CurrentHealth, float MaxHealth)
{
	if(StaminaBar)
	{
		float amount = CurrentHealth / MaxHealth;
		StaminaBar->SetPercent(amount);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "StaminaBar is NULL");
	}
}

void UPlayerHealthBar::SetPostureBarValue(float Current, float Max)
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

void UPlayerHealthBar::BindHealthBar(ACharacter* Character)
{
	if(Character)
	{
		ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character);

		if(PlayerCharacter && PlayerCharacter->GetHealthComponent())
		{
			PlayerCharacter->GetHealthComponent()->OnEntityHealthChanged.AddUObject(this, &UPlayerHealthBar::SetHealthBarValue);
		}

		SetHealthBarValue(1.f, 1.f);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "No Character");
	}
}

void UPlayerHealthBar::BindStaminaBar(ACharacter* Character)
{
	if(Character)
	{
		ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character);

		if(PlayerCharacter && PlayerCharacter->GetSprintComponent())
		{
			PlayerCharacter->GetSprintComponent()->OnEntityStaminaChanged.AddUObject(this, &UPlayerHealthBar::SetStaminaBarValue);
		}

		SetStaminaBarValue(1.f, 1.f);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "No Character");
	}
}

void UPlayerHealthBar::BindPostureBar(ACharacter* Character)
{
	if(Character)
	{
		ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character);
		
		if(PlayerCharacter && PlayerCharacter->GetParryComponent())
		{
			PlayerCharacter->GetParryComponent()->OnPostureChanged.AddUObject(this, &UPlayerHealthBar::SetPostureBarValue);
		}

		SetPostureBarValue(1.f, 1.f);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "No Character");
	}
}