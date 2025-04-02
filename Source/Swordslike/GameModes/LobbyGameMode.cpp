#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Player/SwordslikeCharacter.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	const int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	if(NewPlayer)
	{
		if(APlayerState* CustomPlayer = Cast<APlayerState>(NewPlayer))
		{
			// CustomPlayer->PlayerName = GameState.Get()->PlayerArray[GameState.Get()->PlayerArray.Num() - 1]->GetPlayerName();
			CustomPlayer->SetPlayerName(FString::Printf(TEXT("Player %d"), GameState.Get()->PlayerArray.Num()));
		}
	}
	
	if(NumberOfPlayers == RequiredPlayersCount)
	{
		if(UWorld* World = GetWorld())
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString(TEXT("/Game/Levels/Level_FirstLevel?listen")));
		}
	}
}
