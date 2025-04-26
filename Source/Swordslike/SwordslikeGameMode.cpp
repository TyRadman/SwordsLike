
#include "SwordslikeGameMode.h"

#include "GameplayGameState.h"
#include "GameplayPlayerState.h"
#include "Player/MainPlayerState.h"
#include "UI/HUD/HUDManager.h"
#include "UnitControllers/Player/SwordslikeCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASwordslikeGameMode::ASwordslikeGameMode()
{
	HUDClass = AHUDManager::StaticClass();

	PlayerStateClass = AGameplayPlayerState::StaticClass();

	GameStateClass = AGameplayGameState::StaticClass(); 
	
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/BP_MainPlayerCharacter"));
	
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
