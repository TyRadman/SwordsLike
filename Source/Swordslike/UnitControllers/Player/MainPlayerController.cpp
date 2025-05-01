#include "MainPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MainPlayerState.h"
#include "PlayerStartCharacterDataAsset.h"
#include "Swordslike/UI/Lobby/LobbyHUD.h"
#include "Swordslike/UI/Lobby/MainLobbyMenu.h"

void AMainPlayerController::ClientReset_Implementation()
{
	Super::ClientReset_Implementation();

	if (IsLocalController() && GetHUD() == nullptr && HUDToUse)
	{
		ClientSetHUD(HUDToUse);
	}
}

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();
}
