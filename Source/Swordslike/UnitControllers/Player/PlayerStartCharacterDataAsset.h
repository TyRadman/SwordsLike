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
	FString CharacterName;
	
	UPROPERTY(EditAnywhere, Category=Stats)
	float StartingHealthPoints;
	UPROPERTY(EditAnywhere, Category=Stats)
	float StartingStamina;
	UPROPERTY(EditAnywhere, Category=Stats)
	float StartingPosture;
	
	UPROPERTY(EditAnywhere, Category=Movement)
	float MovementSpeed = 100;
	UPROPERTY(EditAnywhere, Category=Movement)
	float SprintSpeed = 100;
	UPROPERTY(EditAnywhere, Category=Movement)
	float JumpHeight = 100;

	UPROPERTY(EditAnywhere, Category=Visuals)
	UTexture2D* CharacterTexture;
	UPROPERTY(EditAnywhere, Category=Visuals)
	USkeletalMesh* CharacterSkeletalMesh;
};
