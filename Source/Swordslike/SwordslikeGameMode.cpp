// Copyright Epic Games, Inc. All Rights Reserved.

#include "SwordslikeGameMode.h"
#include "UnitControllers/Player/SwordslikeCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASwordslikeGameMode::ASwordslikeGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_ThirdPersonCharacter"));
	
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
