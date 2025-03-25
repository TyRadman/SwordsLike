#include "LockableTargetComponent.h"
#include "Player/SwordslikeCharacter.h"
#include "Swordslike/UI/WorldUIElements/OverheadHealthBarWidget.h"

ULockableTargetComponent::ULockableTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULockableTargetComponent::InitEntityComponent(ACharacter* Character)
{
	if(Character)
	{
		if(ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character))
		{
			if(UOverheadHealthBarWidget* HUD = PlayerCharacter->GetOverHeadHUDComponent())
			{
				// TODO: check if it's the owner, and don't enable it if so
				OnLockableLocked.AddUObject(HUD, &UOverheadHealthBarWidget::Show);
				OnLockableUnlocked.AddUObject(HUD, &UOverheadHealthBarWidget::Hide);
			}
		}
	}
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

void ULockableTargetComponent::OnDeath()
{
	IsValid = false;
}

// server methods must be removed
void ULockableTargetComponent::Server_OnUnlocked_Implementation()
{
	if(OnLockableUnlocked.IsBound())
	{
		OnLockableUnlocked.Broadcast();
	}
}

void ULockableTargetComponent::Server_OnLocked_Implementation()
{
	if(OnLockableLocked.IsBound())
	{
		OnLockableLocked.Broadcast();
	}
}

