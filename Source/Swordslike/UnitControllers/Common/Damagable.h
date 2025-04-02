#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damagable.generated.h"

struct FDamageInfo;

UINTERFACE(Blueprintable)
class SWORDSLIKE_API UDamagable : public UInterface
{
	GENERATED_BODY()
};

class SWORDSLIKE_API IDamagable
{
	GENERATED_BODY()

public:
	virtual void TakeDamage(const FDamageInfo& Damage) = 0;
	virtual bool IsAlive() = 0;
	virtual bool IsInvincible() const = 0;
};
