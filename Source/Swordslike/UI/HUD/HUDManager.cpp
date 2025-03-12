// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDManager.h"

#include "SprintComponent.h"
#include "Blueprint/UserWidget.h"
#include "HealthBars/PlayerHealthBar.h"
#include "Player/PlayerHealthComponent.h"
#include "Player/SwordslikeCharacter.h"


AHUDManager::AHUDManager()
{
	static ConstructorHelpers::FClassFinder<UUserWidget>HealthBarWidgetClass(TEXT("/Game/UI/HUD/HealthBars/PlayerHealthBar/WBP_HealthBar"));

	if(HealthBarWidgetClass.Class)
	{
		PlayerStats = CreateWidget<UPlayerHealthBar>(GetWorld(), HealthBarWidgetClass.Class);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Widget Class not found"));
	}
}

void AHUDManager::BeginPlay()
{
	Super::BeginPlay();

	CreateHealthBar();
}

void AHUDManager::CreateHealthBar()
{
	if(PlayerStats)
	{
		PlayerStats->AddToViewport();
	}
}

void AHUDManager::BindHealthBar(ACharacter* Character)
{
	if(Character && Character->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character);

		if(PlayerStats)
		{
			if(PlayerCharacter && PlayerCharacter->GetHealthComponent())
			{
				PlayerCharacter->GetHealthComponent()->OnEntityHealthChanged.AddUObject(PlayerStats, &UPlayerHealthBar::SetHealthBarValue);
			}

			PlayerStats->SetHealthBarValue(1.f, 1.f);
		}
	}
}

void AHUDManager::BindStaminaBar(ACharacter* Character)
{
	if(Character && Character->GetLocalRole() == ROLE_AutonomousProxy)
	{
		ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character);

		if(PlayerStats)
		{
			if(PlayerCharacter && PlayerCharacter->GetHealthComponent())
			{
				PlayerCharacter->GetSprintComponent()->OnEntityStaminaChanged.AddUObject(PlayerStats, &UPlayerHealthBar::SetStaminaBarValue);
			}

			PlayerStats->SetStaminaBarValue(1.f, 1.f);
		}
	}
}
