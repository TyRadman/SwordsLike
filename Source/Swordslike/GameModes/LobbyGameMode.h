#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class SWORDSLIKE_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ALobbyGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override;

	UPROPERTY(EditDefaultsOnly)
	int32 RequiredPlayersCount = 3;
	UPROPERTY(EditDefaultsOnly)
	FString LevelName = "Level_FirstLevel";

	const FString LevelsRootPath = "/Game/Levels/";

private:
	void OnPlayerReady();
	void OnPlayerNotReady();
	int32 ReadyPlayersCount = 0;
};
