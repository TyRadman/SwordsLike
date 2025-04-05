#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "Player/LobbyPlayerPawn.h"
#include "Player/MainPlayerState.h"
#include "Swordslike/SwordslikeGameInstance.h"
#include "Swordslike/UI/Lobby/LobbyHUD.h"


class USwordslikeGameInstance;

ALobbyGameMode::ALobbyGameMode()
{
	HUDClass = ALobbyHUD::StaticClass();
	
	// DefaultPawnClass = ALobbyPlayerPawn::StaticClass();
	
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/PlayerSettings/BP_LobbyPlayerPawn"));
	
	if (PlayerPawnBPClass.Class != nullptr)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	
	PlayerStateClass = AMainPlayerState::StaticClass();
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	
	if (AMainPlayerState* PS = NewPlayer->GetPlayerState<AMainPlayerState>())
	{
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("MainPlayerState is valid"));

		if (USwordslikeGameInstance* GI = Cast<USwordslikeGameInstance>(GetGameInstance()))
		{
			// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("SwordslikeGameInstance is valid"));

			if (GI->PlayerCharactersData.IsValidIndex(0))
			{
				// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Character data index 0 is valid"));

				if (GI->PlayerCharactersData[0].IsNull())
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Character data at index 0 is NULL"));
				}
				else
				{
					// const FString AssetName = GI->PlayerCharactersData[0].ToString();
					// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Assigning character: %s"), *AssetName));

					PS->SetCharacterDataAsset(GI->PlayerCharactersData[0].LoadSynchronous());
				}
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Character data index 0 is invalid"));
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("GameInstance is not SwordslikeGameInstance"));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PlayerState is not AMainPlayerState"));
	}

	
	if(NumberOfPlayers == RequiredPlayersCount)
	{
		if(UWorld* World = GetWorld())
		{
			bUseSeamlessTravel = true;
			// World->ServerTravel(FString(TEXT("/Game/Levels/Level_FirstLevel?listen")));
			const FString LevelFullPath = FString::Printf(TEXT("%s%s?listen"), *LevelsRootPath, *LevelName);
			// GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, *LevelFullPath);
			World->ServerTravel(LevelFullPath);
		}
	}
}
