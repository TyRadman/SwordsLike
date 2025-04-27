#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlayerStartCharacterDataAsset.generated.h"

class UCharacterAnimationProfileDA;
/**
 * Holds the data related to the character selection phase like the starting player character and starting weapons.
 */
UCLASS(Blueprintable)
class SWORDSLIKE_API UPlayerStartCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, AssetRegistrySearchable)
	FString CharacterName;
	
	UPROPERTY(EditAnywhere, Category=Stats, AssetRegistrySearchable)
	float StartingHealthPoints = 50.f;
	UPROPERTY(EditAnywhere, Category=Stats, AssetRegistrySearchable)
	float StartingStamina = 10.;
	UPROPERTY(EditAnywhere, Category=Stats, AssetRegistrySearchable)
	float StartingPosture = 10.f;
	UPROPERTY(EditAnywhere, Category=Stats, AssetRegistrySearchable)
	float StartingDamageMultiplier = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category=Stats, AssetRegistrySearchable)
	int SpiritsCost = 1;
	
	UPROPERTY(EditAnywhere, Category=Movement, AssetRegistrySearchable)
	float MovementSpeed = 100;
	UPROPERTY(EditAnywhere, Category=Movement, AssetRegistrySearchable)
	float SprintSpeed = 100;
	UPROPERTY(EditAnywhere, Category=Movement, AssetRegistrySearchable)
	float JumpHeight = 100;

	UPROPERTY(EditAnywhere, Category=Visuals)
	UTexture2D* CharacterTexture;
	UPROPERTY(EditAnywhere, Category=Visuals)
	USkeletalMesh* CharacterSkeletalMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Visuals)
	UCharacterAnimationProfileDA* AnimationData;
};
