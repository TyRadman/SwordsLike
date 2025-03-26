// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Damagable.h"
#include "IEntityComponent.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "Common/DamageInfo.h"
#include "BaseHealthComponent.generated.h"

class FLifetimeProperty;

DECLARE_MULTICAST_DELEGATE_OneParam(HealthDelegate, const FDamageInfo& DamageInfo);
DECLARE_MULTICAST_DELEGATE(DeathDelegate);
DECLARE_MULTICAST_DELEGATE_TwoParams(OnEntityHealthChangedDelegate, float CurrentHP, float MaxHP);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API UBaseHealthComponent : public UMyActorComponent, public IDamagable, public IIEntityComponent
{
	GENERATED_BODY()

public:	
	UBaseHealthComponent();
	virtual void InitEntityComponent(ACharacter* Character) override;
	UFUNCTION(Server, Reliable)
	void Server_SetStartingHealth(float Health);

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	

	// IDamagable function
	virtual void TakeDamage(const FDamageInfo& Info) override;

	virtual void AddToCurrentHealth(const FDamageInfo& DamageInfo);
	UFUNCTION(Server, Reliable)
	void Server_AddToCurrentHealth(const FDamageInfo& DamageInfo);
	void PerformAddToCurrentHealth(const FDamageInfo& DamageInfo);
	
	virtual void SetMaxHealth(float MaxHP);
	
	FORCEINLINE virtual float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE virtual void FullyChargeHealth() { CurrentHealth = MaxHealth; }
	virtual void OnDeath();
	FORCEINLINE virtual bool IsAlive() override { return bIsAlive; } 

	/**
	 * Called upon the entity's death. Use AddUObject for now.
	 */
	DeathDelegate OnEntityDeath;

	/**
	 * Called every time the entity's health amount changes.
	 */
	OnEntityHealthChangedDelegate OnEntityHealthChanged;

	HealthDelegate OnEntityHit;

	FORCEINLINE void SetIsInvincible(const bool IsInvincible) { bIsInvincible = IsInvincible;}
	FORCEINLINE bool IsInvincible() const { return bIsInvincible; }
	

protected:
	UPROPERTY(ReplicatedUsing = OnRep_MaxHealth)
	float MaxHealth;
	UFUNCTION()
	void OnRep_MaxHealth();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth)
	float CurrentHealth;
	UFUNCTION()
	void OnRep_CurrentHealth();

	UPROPERTY(Replicated)
	bool bIsInvincible = false;

	UPROPERTY(ReplicatedUsing = OnRep_bIsAlive)
	bool bIsAlive = true;
	UFUNCTION()
	void OnRep_bIsAlive();
	
#pragma region Take Damage
	UFUNCTION(Server, Reliable)
	virtual void Server_TakeDamage(const FDamageInfo& Info);
#pragma endregion

#pragma region Set Max Health
	UFUNCTION(Server, Reliable)
	virtual void Server_SetMaxHealth(float MaxHP);
	
#pragma endregion

	void ApplyMaxHealth(const float MaxHP);
	void ApplyDamage(const FDamageInfo& DamageInfo);
};
