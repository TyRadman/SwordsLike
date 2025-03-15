#include "PlayerHealthBar.h"

#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "BaseParryComponent.h"
#include "SprintComponent.h"
#include "Player/SwordslikeCharacter.h"
#include "Player/PlayerHealthComponent.h"

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
	}
}

void UPlayerHealthBar::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPlayerHealthBar::BindHealthBar(ACharacter* Character)
{
	if(Character && Character->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character);

		if(PlayerCharacter && PlayerCharacter->GetHealthComponent())
		{
			PlayerCharacter->GetHealthComponent()->OnEntityHealthChanged.AddUObject(this, &UPlayerHealthBar::SetHealthBarValue);
		}

		SetHealthBarValue(1.f, 1.f);
	}
}

void UPlayerHealthBar::BindStaminaBar(ACharacter* Character)
{
	if(Character && Character->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character);

		if(PlayerCharacter && PlayerCharacter->GetSprintComponent())
		{
			PlayerCharacter->GetSprintComponent()->OnEntityStaminaChanged.AddUObject(this, &UPlayerHealthBar::SetStaminaBarValue);
		}

		SetStaminaBarValue(1.f, 1.f);
	}
}

void UPlayerHealthBar::BindPostureBar(ACharacter* Character)
{
	if(Character && Character->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character);
		
		if(PlayerCharacter && PlayerCharacter->GetParryComponent())
		{
			PlayerCharacter->GetParryComponent()->OnPostureChanged.AddUObject(this, &UPlayerHealthBar::SetPostureBarValue);
		}

		SetPostureBarValue(1.f, 1.f);
	}
}