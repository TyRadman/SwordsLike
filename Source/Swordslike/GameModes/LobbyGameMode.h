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
	int32 RequiredPlayersCount = 2;
	UPROPERTY(EditDefaultsOnly)
	FString LevelName;

	const FString LevelsRootPath = "/Game/Levels/";
};
