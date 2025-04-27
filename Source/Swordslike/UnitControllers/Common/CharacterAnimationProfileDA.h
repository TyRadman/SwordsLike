#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterAnimationProfileDA.generated.h"

 /**
 * Holds Data for the character's movement animation.
 */
UCLASS(Blueprintable)
class SWORDSLIKE_API UCharacterAnimationProfileDA : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	UBlendSpace* StrafeBlendSpace;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	UAnimSequence* RunSequence;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(AllowPrivateAccess=true))
	UAnimSequence* CombatRunSequence;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,meta=(AllowPrivateAccess=true))
	float MinWalkSpeed = 100.0f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,meta=(AllowPrivateAccess=true))
	float MinRunSpeed = 200.0f;
};
