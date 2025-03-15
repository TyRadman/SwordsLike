// Fill out your copyright notice in the Description page of Project Settings.


#include "HUDManager.h"
#include "MasterHUD.h"
#include "Blueprint/UserWidget.h"


AHUDManager::AHUDManager()
{
	static ConstructorHelpers::FClassFinder<UUserWidget>MasterHUDWidgetClass(TEXT("/Game/UI/HUD/WBP_Master"));

	if(MasterHUDWidgetClass.Class)
	{
		MasterHUD = CreateWidget<UMasterHUD>(GetWorld(), MasterHUDWidgetClass.Class);
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
	if(MasterHUD)
	{
		MasterHUD->AddToViewport();
	}
}
