// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "BaseCombatComponent.generated.h"

class UAnimInstance;
class UWeaponHandlerComponent;

DECLARE_MULTICAST_DELEGATE(RollDelegate);

UCLASS(Abstract)
class SWORDSLIKE_API UBaseCombatComponent : public UMyActorComponent
{
	GENERATED_BODY()

public:	
	UBaseCombatComponent();
	
	UFUNCTION(Server, Reliable)
	void Server_PlayMontage();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage();
	
	UFUNCTION(Server, Reliable)
	void Server_ForceStopAttack();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ForceStopAttack();

protected:
	virtual void BeginPlay() override;
	
	virtual void PlayAttackAnimation();

	
	UFUNCTION()
	virtual void OnAttackEnded(UAnimMontage* Anim, bool bInterrupted);

public:
	// TODO: To be move to the EntityAnimation component
	// UPROPERTY(EditDefaultsOnly, Category="Animations")
	// TObjectPtr<UAnimMontage> AttackAnimationMontage;
	UPROPERTY(EditDefaultsOnly, Category="Animations")
	TObjectPtr<UAnimMontage> AttackInterruptionMontage;

	void SetWeaponHandler(TObjectPtr<UWeaponHandlerComponent> Handler);
	FORCEINLINE UWeaponHandlerComponent* GetWeaponHandler() const {return WeaponHandler;}

	/**
	 * Broadcasts when the entity performs a roll.
	 */
	RollDelegate OnEntityRolled;
	RollDelegate OnEntityRollFinished;
	void SetCanRoll(bool CanRoll);
	
	virtual void ForceStopAttack();
	virtual void PerformForceStop();

	void AllowInput();
	void DisableInput();

private:
	TObjectPtr<UAnimInstance> AnimInstance;

protected:
	bool bCanAttack = false;
	bool bIsAttacking = false;
	bool bIsPerformingCombo = false;
	bool bCanPerformCombo = false;

	bool bCanRoll = false;
	bool bIsRolling = false;
	FTimerHandle RollTimer;
	float RollDuration = 0.7f;
	
	TObjectPtr<UWeaponHandlerComponent> WeaponHandler;

	// EXTERNALS
public:
	virtual void AttackAction();
	
	void EnableRoll();
	void DisableRoll();
	// ROLLING
	virtual void Roll();
	virtual void RollRecover();
	void SetRollDuration(float Duration);
	
};
