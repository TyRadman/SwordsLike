#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Player/SwordslikeCharacter.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	if(NewPlayer)
	{
		if(ASwordslikeCharacter* CustomPlayer = Cast<ASwordslikeCharacter>(NewPlayer))
		{
			CustomPlayer->PlayerName = GameState.Get()->PlayerArray[GameState.Get()->PlayerArray.Num() - 1]->GetPlayerName();
		}
	}
	
	if(NumberOfPlayers == 2)
	{
		if(UWorld* World = GetWorld())
		{
			bUseSeamlessTravel = true;
			World->ServerTravel(FString(TEXT("/Game/Levels/Level_Arena_02?listen")));
		}
	}
}
