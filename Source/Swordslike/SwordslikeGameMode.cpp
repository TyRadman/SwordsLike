// Copyright Epic Games, Inc. All Rights Reserved.

#include "SwordslikeGameMode.h"

#include "UI/HUD/HUDManager.h"
#include "UnitControllers/Player/SwordslikeCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASwordslikeGameMode::ASwordslikeGameMode()
{
	HUDClass = AHUDManager::StaticClass();
	
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_MainPlayerCharacter"));
	
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
