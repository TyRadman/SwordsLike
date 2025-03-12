#include "LockableTargetComponent.h"

#include "DamageInfo.h"

ULockableTargetComponent::ULockableTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULockableTargetComponent::OnLocked()
{
	if(OnLockableLocked.IsBound())
	{
		OnLockableLocked.Broadcast();
	}
}

void ULockableTargetComponent::OnUnlocked()
{
	if(OnLockableUnlocked.IsBound())
	{
		OnLockableUnlocked.Broadcast();
	}
}

bool ULockableTargetComponent::IsValidTarget() const
{
	return IsValid;
}

void ULockableTargetComponent::OnDeath(const FDamageInfo& DamageInfo)
{
	IsValid = false;
}

