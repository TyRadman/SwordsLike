#pragma once

#include "CoreMinimal.h"
#include "DamageInfo.generated.h"

enum class EHitType : uint8;

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

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FVector ImpactLocation;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	EHitType HitType;
};
