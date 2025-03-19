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
DECLARE_MULTICAST_DELEGATE_TwoParams(OnEntityHealthChangedDelegate, float CurrentHP, float MaxHP);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API UBaseHealthComponent : public UMyActorComponent, public IDamagable, public IIEntityComponent
{
	GENERATED_BODY()

public:	
	UBaseHealthComponent();
	virtual void InitEntityComponent(ACharacter* Character) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	

	// IDamagable function
	virtual void TakeDamage(const FDamageInfo& Info) override;

	virtual void AddToCurrentHealth(const FDamageInfo& DamageInfo);
	virtual void SetMaxHealth(float MaxHP);
	
	virtual float GetMaxHealth();
	virtual void FullyChargeHealth();
	virtual void OnDeath();
	virtual bool IsAlive() override;

	/**
	 * Called upon the entity's death. Use AddUObject for now.
	 */
	HealthDelegate OnEntityDeath;

	/**
	 * Called every time the entity's health amount changes.
	 */
	OnEntityHealthChangedDelegate OnEntityHealthChanged;

	HealthDelegate OnEntityHit;

	void SetIsInvincible(bool IsInvincible);
	bool IsInvincible() const;
	

protected:
	UPROPERTY(EditAnywhere)
	float MaxHealth;

	float CurrentHealth;

	UPROPERTY(Replicated)
	bool bIsInvincible = false;

	UPROPERTY(Replicated)
	bool bIsAlive = true;
	
#pragma region Take Damage
	UFUNCTION(Server, Reliable)
	virtual void ServerTakeDamage(const FDamageInfo& Info);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastTakeDamage(const FDamageInfo& Info);
#pragma endregion

#pragma region Set Max Health
	UFUNCTION(Server, Reliable)
	virtual void ServerSetMaxHealth(float MaxHP);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSetMaxHealth(float MaxHP);
#pragma endregion

	void ApplyMaxHealth(float MaxHP);
	void ApplyDamage(const FDamageInfo& DamageInfo);
};
