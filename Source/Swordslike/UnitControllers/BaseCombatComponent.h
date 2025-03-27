// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "BaseCombatComponent.generated.h"

class ASwordslikeCharacter;
class UAnimInstance;
class UWeaponHandlerComponent;

DECLARE_MULTICAST_DELEGATE(RollDelegate);

UCLASS(Abstract)
class SWORDSLIKE_API UBaseCombatComponent : public UMyActorComponent, public IIEntityComponent
{
	GENERATED_BODY()

public:	
	UBaseCombatComponent();
	
	
	UFUNCTION(Server, Reliable)
	void Server_ForceStopAttack();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ForceStopAttack();
	
	virtual void InitEntityComponent(ACharacter* Character);

protected:
	void PlayNextAnimation();
	UFUNCTION(Server, Reliable)
	void Server_PlayMontage();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage();
	void PerformPlayAttackAnimation();

	int32 ComboCount = 0;
	
	UFUNCTION()
	virtual void OnAttackEnded(UAnimMontage* Anim, bool bInterrupted);

	bool bIsEndOfCombo = true;
	
	ASwordslikeCharacter* PlayerCharacter;

public:
	UPROPERTY(EditDefaultsOnly, Category="Animations")
	TObjectPtr<UAnimMontage> AttackInterruptionMontage;

	void SetWeaponHandler(const TObjectPtr<UWeaponHandlerComponent>& Handler);
	FORCEINLINE UWeaponHandlerComponent* GetWeaponHandler() const {return WeaponHandler;}

	/**
	 * Broadcasts when the entity performs a roll.
	 */
	RollDelegate OnEntityRolled;
	RollDelegate OnEntityRollFinished;
	FORCEINLINE void SetCanRoll(const bool CanRoll) {bCanRoll = CanRoll;}
	
	virtual void ForceStopAttack();
	virtual void PerformForceStop();

	void AllowInput();
	void PerformNextAttack();
	void DisableInput();

private:
	TObjectPtr<UAnimInstance> AnimInstance;

protected:
	bool bCanAttack = false;
	bool bIsAttacking = false;
	bool bIsPerformingCombo = false;
	bool bIdealNextAttackPointPassed = false;
	bool bCanPerformCombo = false;

	bool bCanRoll = false;
	bool bIsRolling = false;
	FTimerHandle RollTimer;
	float RollDuration = 0.7f;
	
	TObjectPtr<UWeaponHandlerComponent> WeaponHandler;

	// EXTERNALS
public:
	virtual void AttackAction();
	
	FORCEINLINE void EnableRoll() { bCanRoll = true; }
	FORCEINLINE void DisableRoll() { bCanRoll = false; }
	// ROLLING
	virtual void Roll();
	virtual void RollRecover();
	FORCEINLINE void SetRollDuration(const float Duration) {RollDuration = Duration;}
	
};
