#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Components/ActorComponent.h"
#include "LockableTargetComponent.generated.h"

struct FDamageInfo;

DECLARE_MULTICAST_DELEGATE(LockableTargetDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API ULockableTargetComponent : public USceneComponent, public IIEntityComponent
{
	GENERATED_BODY()

public:	
	ULockableTargetComponent();
	virtual void InitEntityComponent(ACharacter* Character) override;

	LockableTargetDelegate OnLockableLocked;
	LockableTargetDelegate OnLockableUnlocked;

	void OnLocked();
	void OnUnlocked();
	void OnDeath(const FDamageInfo& DamageInfo);
	
	FORCEINLINE bool IsValidTarget() const { return IsValid; }

private:
	/**
	 * The target is valid if it's alive, or is targetable by the locker.
	 */
	bool IsValid = true;
};
