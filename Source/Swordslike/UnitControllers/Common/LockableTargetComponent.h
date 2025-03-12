// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockableTargetComponent.generated.h"

struct FDamageInfo;

DECLARE_MULTICAST_DELEGATE(LockableTargetDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API ULockableTargetComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	ULockableTargetComponent();

	LockableTargetDelegate OnLockableLocked;
	LockableTargetDelegate OnLockableUnlocked;

	void OnLocked();
	void OnUnlocked();
	bool IsValidTarget() const;
	void OnDeath(const FDamageInfo& DamageInfo);

private:
	/**
	 * The target is valid if it's alive, or is targetable by the locker.
	 */
	bool IsValid = true;
};
