#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerStartCharacterDataAsset.generated.h"

/**
 * Holds the data related to the character selection phase like the starting player character and starting weapons.
 */
UCLASS()
class SWORDSLIKE_API UPlayerStartCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	USkeletalMesh* CharacterSkeletalMesh;
	
	UPROPERTY(EditAnywhere)
	float StartingHealthPoints;
	UPROPERTY(EditAnywhere)
	float StartingStamina;
	UPROPERTY(EditAnywhere)
	float StartingPosture;

	UPROPERTY(EditAnywhere)
	UTexture2D* CharacterTexture;
};
