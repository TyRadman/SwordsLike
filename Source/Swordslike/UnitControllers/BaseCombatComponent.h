// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "BaseCombatComponent.generated.h"

UENUM()
enum class EComboState : uint8
{
	Idle = 0,
	Attacking = 1,
	ComboWindowOpen = 2,
	ComboQueued = 3,
	Ending = 4,
	LastSecondComboWindowOpen = 5
};

class UWeaponAttackIndicatorWidget;
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
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void PlayNextAnimation();
	UFUNCTION(Server, Reliable)
	void Server_PlayMontage();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage();
	void PerformPlayAttackAnimation();

	int32 ComboCount = 0;
	
	ASwordslikeCharacter* PlayerCharacter;

public:
	UFUNCTION()
	virtual void OnAttackEnded(UAnimMontage* Anim, bool bInterrupted);
	
	UPROPERTY(EditDefaultsOnly, Category="Animations")
	TObjectPtr<UAnimMontage> AttackInterruptionMontage;

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

	void StartAttackWarning(const float Duration, const float AnticipationSpeedMultiplier = 1.0f);
	UFUNCTION(Server, Reliable)
	void Server_StartWarning(const float Duration, const float AnticipationSpeedMultiplier = 1.0f);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartWarning(const float Duration, const float AnticipationSpeedMultiplier = 1.0f);
	void PerformStartAttackWarninig(const float Duration, const float AnticipationSpeedMultiplier = 1.0f);
	
	void EndAttackWarning();
	UFUNCTION(Server, Reliable)
	void Server_EndAttackWarning();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndAttackWarning();
	void PerformEndAttackWarning();
	const float AttackWarningRadius = 750.f;

private:
	TObjectPtr<UAnimInstance> AnimInstance;
	UAnimMontage* CurrentAttackMontage;

protected:
	FTimerHandle RollTimer;

	bool bCanRoll = false;
	bool bIsRolling = false;

	float RollDuration = 0.7f;
	
	TObjectPtr<UWeaponHandlerComponent> WeaponHandler;
	UWeaponAttackIndicatorWidget* AttackIndicatorWidget;
	
	EComboState ComboState = EComboState::Idle;

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
