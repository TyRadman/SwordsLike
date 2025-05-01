#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Player/PlayerStartCharacterDataAsset.h"
#include "SwordslikeGameInstance.generated.h"

class UPlayerStartCharacterDataAsset;

/**
 * The game's default game instance
 */
UCLASS()
class SWORDSLIKE_API USwordslikeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=CharactersData)
	TArray<TSoftObjectPtr<UPlayerStartCharacterDataAsset>> PlayerCharactersData;

	/**
	 * Returns the selected character data in the lobby level. If a value isn't provided, then it returns a default value.
	 * @return The character data to apply to the player.
	 */
	FORCEINLINE UPlayerStartCharacterDataAsset* GetPlayerCharacterData()
	{
		if(LocalData)
		{
			return LocalData;
		}

		if(DefaultCharacterData)
		{
			return DefaultCharacterData;
		}

		return nullptr;
	}

	FORCEINLINE void SetLocalCharacterData(UPlayerStartCharacterDataAsset* NewCharacterData)
	{
		LocalData = NewCharacterData;
	}

	UPROPERTY(EditDefaultsOnly)
	FString PlayerName = FString("Dummy Name");
	
private:
	UPROPERTY(EditAnywhere, Category=CharactersData, meta=(AllowPrivateAccess=true))
	UPlayerStartCharacterDataAsset* DefaultCharacterData;
	UPlayerStartCharacterDataAsset* LocalData;
};
