#include "LobbyGameState.h"

#include "Player/MainPlayerState.h"

void ALobbyGameState::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);

	if(PlayerState)
	{
		if(AMainPlayerState* State = Cast<AMainPlayerState>(PlayerState))
		{
			PlayersCounter++;
			State->StateIndex = PlayersCounter;	
		}
	}
}
