#include "BaseHealthComponent.h"

#include "BaseEntityData.h"
#include "DamageInfo.h"
#include "LockableTargetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/SwordslikeCharacter.h"
#include "Swordslike/UI/HUD/MasterHUD.h"
#include "Swordslike/UI/HUD/HealthBars/PlayerHealthBar.h"
#include "Swordslike/UI/WorldUIElements/OverheadHealthBarWidget.h"

UBaseHealthComponent::UBaseHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UBaseHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBaseHealthComponent, bIsInvincible);
	DOREPLIFETIME(UBaseHealthComponent, bIsAlive);
	DOREPLIFETIME(UBaseHealthComponent, CurrentHealth);
	DOREPLIFETIME(UBaseHealthComponent, MaxHealth);
}

void UBaseHealthComponent::InitEntityComponent(ACharacter* Character)
{
	if(ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character))
	{
		OnEntityDeath.AddUObject(PlayerCharacter, &ASwordslikeCharacter::OnDeath);

		if(ULockableTargetComponent* Lockable = PlayerCharacter->GetLockableTargetComponent())
		{
			OnEntityDeath.AddUObject(Lockable, &ULockableTargetComponent::OnDeath);
		}

		// Hit react animation
		OnEntityHit.AddUObject(PlayerCharacter, &ASwordslikeCharacter::OnCharacterHit);

		// TODO: only if the entity is not an owner
		if(UOverheadHealthBarWidget* HUD = PlayerCharacter->GetOverHeadHUDComponent())
		{
			OnEntityHealthChanged.AddUObject(HUD, &UOverheadHealthBarWidget::SetHealthBarValue);
		}

		if(const UMasterHUD* MasterHUD = PlayerCharacter->GetMasterHUD())
		{
			if(UPlayerHealthBar* PlayerHUD = MasterHUD->GetStatsHUD())
			{
			PrintOnScreen_Local(FString::Printf(TEXT("Subscribed successfully")));
				OnEntityHealthChanged.AddUObject(PlayerHUD, &UPlayerHealthBar::SetHealthBarValue);
				PlayerHUD->SetHealthBarValue(1.f, 1.f);
			}
		}
		else
		{
			PrintOnScreen_Local(FString::Printf(TEXT("No Master HUD")));
			return;
		}

		const float StartingHealth = PlayerCharacter->GetPlayerStats()->MaxHealthPoints;
		SetMaxHealth(StartingHealth);
		FDamageInfo DamageInfo;
		DamageInfo.Damage = -StartingHealth;
		AddToCurrentHealth(DamageInfo);
	}
}

void UBaseHealthComponent::TakeDamage(const FDamageInfo& Info)
{
	if(!HasAuthority())
	{
		Server_TakeDamage(Info);
	}
	else
	{
		ApplyDamage(Info);
	}
}

void UBaseHealthComponent::Server_TakeDamage_Implementation(const FDamageInfo& Info)
{
	ApplyDamage(Info);
}

void UBaseHealthComponent::ApplyDamage(const FDamageInfo& DamageInfo)
{
	if(bIsInvincible)
	{
		return;
	}
	
	OnEntityHit.Broadcast(DamageInfo);
}

void UBaseHealthComponent::SetMaxHealth(float MaxHP)
{
	// PrintOnScreen_Local(FString::Printf(TEXT("CLIENT Max Health: %f"), MaxHP), 20.f);
	if(!HasAuthority())
	{
		Server_SetMaxHealth(MaxHP);
	}
	else
	{
		ApplyMaxHealth(MaxHP);
	}
}

void UBaseHealthComponent::Server_SetMaxHealth_Implementation(float MaxHP)
{
	ApplyMaxHealth(MaxHP);
}

void UBaseHealthComponent::ApplyMaxHealth(const float MaxHP)
{
	MaxHealth = MaxHP;
}

void UBaseHealthComponent::AddToCurrentHealth(const FDamageInfo& DamageInfo)
{
	if(!HasAuthority())
	{
		Server_AddToCurrentHealth(DamageInfo);
	}
	else
	{
		PerformAddToCurrentHealth(DamageInfo);
	}
}

void UBaseHealthComponent::Server_AddToCurrentHealth_Implementation(const FDamageInfo& DamageInfo)
{
	PerformAddToCurrentHealth(DamageInfo);
}

void UBaseHealthComponent::PerformAddToCurrentHealth(const FDamageInfo& DamageInfo)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageInfo.Damage, 0.0f, MaxHealth);
	OnRep_CurrentHealth();

	if (CurrentHealth <= 0)
	{
		// PrintOnScreen_Local(FString::Printf(TEXT("CLIENT (DEATH): %f / %f"), CurrentHealth, MaxHealth));
		bIsAlive = false;
		OnRep_bIsAlive();
	}
}

void UBaseHealthComponent::OnDeath()
{
	
}

void UBaseHealthComponent::OnRep_MaxHealth()
{
	// const float HealthPercentage = CurrentHealth / MaxHealth;
	// CurrentHealth = MaxHealth * HealthPercentage;
}

void UBaseHealthComponent::OnRep_CurrentHealth()
{
	PrintOnScreen(FString::Printf(TEXT("OnRep: %f / %f"), CurrentHealth, MaxHealth));
	OnEntityHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UBaseHealthComponent::OnRep_bIsAlive()
{
	OnEntityDeath.Broadcast();
}
