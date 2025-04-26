#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "GameplayGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerDeath, APlayerState*);
UCLASS()
class SWORDSLIKE_API AGameplayGameState : public AGameState
{
	GENERATED_BODY()

	// Event

public:
	FOnPlayerDeath OnPlayerDeathReported;
	void ReportDeath(APlayerState* DeadPlayer);
	void CheckAlivePlayers();
};
