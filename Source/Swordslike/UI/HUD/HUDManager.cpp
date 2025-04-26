
#include "HUDManager.h"
#include "MasterHUD.h"
#include "Blueprint/UserWidget.h"
#include "Swordslike/UI/Menus/GameOverMenuWidget.h"


AHUDManager::AHUDManager()
{
}

void AHUDManager::BeginPlay()
{
	Super::BeginPlay();

	if(GameOverMenuReference)
	{
		GameOverMenu = CreateWidget<UGameOverMenuWidget>(GetWorld(), GameOverMenuReference);
	}

	if(MasterHUDReference)
	{
		MasterHUD = CreateWidget<UMasterHUD>(GetWorld(), MasterHUDReference);
	}

	CreateHealthBar();
}

void AHUDManager::CreateHealthBar()
{
	if(MasterHUD)
	{
		MasterHUD->AddToViewport();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("NO MASTER WIDGET CLASS"));
	}
}
