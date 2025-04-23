#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "Player/LobbyPlayerPawn.h"
#include "Player/MainPlayerState.h"
#include "Swordslike/SwordslikeGameInstance.h"
#include "Swordslike/UI/Lobby/LobbyHUD.h"
#include "Swordslike/UI/Lobby/PlayerSelectionMenuWidget.h"


class USwordslikeGameInstance;

ALobbyGameMode::ALobbyGameMode()
{
	HUDClass = ALobbyHUD::StaticClass();

	static ConstructorHelpers::FClassFinder<AHUD> CharacterSelectionHUD(TEXT("/Game/UI/Lobby/BP_LobbyHUD"));
	
	if (CharacterSelectionHUD.Class != nullptr)
	{
		HUDClass = CharacterSelectionHUD.Class;
	}
	
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

	// const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
	RequiredPlayersCount = GameState.Get()->PlayerArray.Num();
	
	if (AMainPlayerState* PS = NewPlayer->GetPlayerState<AMainPlayerState>())
	{
		if (USwordslikeGameInstance* GI = Cast<USwordslikeGameInstance>(GetGameInstance()))
		{
			if (GI->PlayerCharactersData.IsValidIndex(0))
			{
					PS->SetCharacterDataAsset(GI->PlayerCharactersData[0].LoadSynchronous());
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

		// register for reading when players are ready
		PS->OnPlayerReady.AddUObject(this, &ALobbyGameMode::OnPlayerReady);
		PS->OnPlayerNotReady.AddUObject(this, &ALobbyGameMode::OnPlayerNotReady);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PlayerState is not AMainPlayerState"));
	}
	
	// cache the biggest possible values based on the data assets
	if (const USwordslikeGameInstance* GI = Cast<USwordslikeGameInstance>(GetGameInstance()))
	{
		TArray<TSoftObjectPtr<UPlayerStartCharacterDataAsset>> Assets = GI->PlayerCharactersData;
		float MaxHealth = KINDA_SMALL_NUMBER;
		float MaxStamina = KINDA_SMALL_NUMBER;
		float MaxPosture = KINDA_SMALL_NUMBER;

		for(TSoftObjectPtr<UPlayerStartCharacterDataAsset> Data : Assets)
		{
			if (Data)
			{
				if(Data->StartingHealthPoints > MaxHealth)
				{
					MaxHealth = Data->StartingHealthPoints;
				}

				if(Data->StartingStamina > MaxStamina)
				{
					MaxStamina = Data->StartingStamina;
				}

				if(Data->StartingPosture > MaxPosture)
				{
					MaxPosture = Data->StartingPosture;
				}
			}
		}

		UPlayerSelectionMenuWidget::MaxHealth = MaxHealth;
		UPlayerSelectionMenuWidget::MaxStamina = MaxStamina;
		UPlayerSelectionMenuWidget::MaxPosture = MaxPosture;
	}
}

void ALobbyGameMode::OnPlayerReady()
{
	ReadyPlayersCount++;
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%d/%d"), ReadyPlayersCount, RequiredPlayersCount));
	if(ReadyPlayersCount == RequiredPlayersCount)
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

void ALobbyGameMode::OnPlayerNotReady()
{
	ReadyPlayersCount--;
}


