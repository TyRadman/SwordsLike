#pragma once

#include "CoreMinimal.h"
#include "DamageInfo.generated.h"

USTRUCT(BlueprintType)
struct SWORDSLIKE_API FDamageInfo
{
	GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Damage = 0.f;

	UPROPERTY(BlueprintReadOnly)
	AActor* Instigator;
	
};
