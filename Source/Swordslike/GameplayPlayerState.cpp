#include "GameplayPlayerState.h"

#include "GameplayGameState.h"

void AGameplayPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (AGameplayGameState* GameState = GetWorld()->GetGameState<AGameplayGameState>())
	{
		GameState->OnPlayerDeathReported.AddLambda([this, GameState](APlayerState* DeadPlayer)
			{
				GameState->CheckAlivePlayers();
			});
	}
}
