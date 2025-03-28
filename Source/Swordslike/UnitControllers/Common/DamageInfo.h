#pragma once

#include "CoreMinimal.h"
#include "DamageInfo.generated.h"

class ASwordslikeCharacter;

USTRUCT(BlueprintType)
struct SWORDSLIKE_API FDamageInfo
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Damage = 0.f;
	
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float PostureDamage = 1.f;

	UPROPERTY(BlueprintReadOnly)
	AActor* DamageInstigator;
	
	UPROPERTY(BlueprintReadOnly)
	ASwordslikeCharacter* DamageInstigatorCharacter;

	FVector ImpactLocation;
};
