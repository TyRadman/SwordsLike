#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Player/PlayerStartCharacterDataAsset.h"
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
	
	float MaxToughness = -1;
	float MaxStamina = -1;
	float MaxPosture = -1;
	float MaxHealth = -1;
	float GetHighestValue(float& MaxValue, float UPlayerStartCharacterDataAsset::* MemberPtr);
	float GetHighestHealthPossible();
	float GetHighestToughnessPossible();
	float GetHighestStaminaPossible();
	float GetHighestPosturePossible();
	
private:
	void OnPlayerReady();
	void OnPlayerNotReady();
	int32 ReadyPlayersCount = 0;
};
