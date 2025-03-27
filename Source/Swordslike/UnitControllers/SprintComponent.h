// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "SprintComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(StaminaDelegate, float CurrentStamina, float MaxStamina);

class ASwordslikeCharacter;
class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API USprintComponent : public UMyActorComponent, public IIEntityComponent
{
	GENERATED_BODY()

public:	
	USprintComponent();

protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	void OnSprintStated();
	void OnSprintEnded();
	
	virtual void InitEntityComponent(ACharacter* Character) override;
	void SetMaxStamina(const float MaxAmount);
	FORCEINLINE float GetCurrentStamina() const { return  CurrentStamina; }
	FORCEINLINE float GetMaxStamina() const {return MaxStamina;}
	void SetCurrentStamina(const float Amount);
	void AddToCurrentStamina(const float Amount);
	void AddToMaxStamina(const float Amount);
	void FullyRefillStamina();

	void OnWeaponHit(AWeapon* Weapon);
	void OnJumped();
	void OnRolled();

	StaminaDelegate OnEntityStaminaChanged;

private:
	// UPROPERTY(Replicated)
	bool bIsSprinting = false;

	TObjectPtr<ASwordslikeCharacter> EntityCharacter;
	void OnStaminaUpdated();

	void StartRefill();
	void StopRefill();

protected:
	FTimerHandle RefillDelayTimer;

	/**
	 * The amount of stamina regenerated per second.
	 */
	const float StaminaRegenerationRate = 1.5f;
	const float DelayBeforeRegeneration = 1.f;
	float CurrentStamina;
	float MaxStamina;
	bool bCanRefill = false;

public:
	static constexpr float JUMP_STAMINA_COST = 1.0f;
	static constexpr float ROLL_STAMINA_COST = 1.0f;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsSprintingValue() const { return bIsSprinting; }

	UFUNCTION(Server, Reliable)
	void Server_SetSprinting(const bool bNewIsSprinting);
	auto PerformStartSprinting() -> void;
	void PerformStopSprinting();
};
