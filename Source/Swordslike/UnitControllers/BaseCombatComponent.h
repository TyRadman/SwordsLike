// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "BaseCombatComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(CombatComponent, Log, All);

UENUM()
enum class EComboState : uint8
{
	None = 0,
	Idle = 1 << 0,
	Attacking = 1 << 1,
	ComboWindowOpen = 1 << 2,
	ComboQueued = 1 << 3,
	Ending = 1 << 4,
	LastSecondComboWindowOpen = 1 << 5,
	Broken = 1 << 6
};

/**
 * Mark the combo state as a bitwise flag enum.
 * @param Lhs 
 * @param Rhs 
 * @return 
 */
ENUM_CLASS_FLAGS(EComboState);

class UWeaponAttackIndicatorWidget;
class ASwordslikeCharacter;
class UAnimInstance;
class UWeaponHandlerComponent;

DECLARE_MULTICAST_DELEGATE(CombatEvent);

UCLASS(Abstract)
class SWORDSLIKE_API UBaseCombatComponent : public UMyActorComponent, public IIEntityComponent
{
	GENERATED_BODY()

public:	
	UBaseCombatComponent();
	
	
	UFUNCTION(Server, Reliable)
	void Server_ForceStopAttack(bool bIsInterruptedAttack);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ForceStopAttack(bool bIsInterruptedAttack);
	
	virtual void InitEntityComponent(ACharacter* Character);

protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void PlayNextAnimation();
	UFUNCTION(Server, Reliable)
	void Server_PlayMontage(UAnimMontage* Montage);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(UAnimMontage* Montage);
	void PerformPlayAttackAnimation(UAnimMontage* Montage);

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
	CombatEvent OnEntityRolled;
	CombatEvent OnEntityRollFinished;
	CombatEvent OnForceStopAttack;
	FORCEINLINE void SetCanRoll(const bool CanRoll) {bCanRoll = CanRoll;}

	UFUNCTION(Client, Reliable)
	void Client_ForceStopAttack(bool bIsInterruptedAttack);
	void ForceStopAttack(bool bIsInterruptedAttack);
	void PerformForceStopAttack(bool bIsInterruptedAttack);
	void PerformForceStop(bool bIsInterrupted);
	virtual void PlayInterruptionAnimation(bool bIsInterruptedAttack);

	void AllowInput();
	void PerformNextAttack();
	void DisableInput();

	void StartAttackWarning(const float Duration, const float AnticipationSpeedMultiplier = 1.0f);
	UFUNCTION(Server, Reliable)
	void Server_StartWarning(const float Duration, const float AnticipationSpeedMultiplier, UAnimMontage* Montage);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartWarning(const float Duration, const float AnticipationSpeedMultiplier, UAnimMontage* Montage);
	void PerformStartAttackWarninig(const float Duration, const float AnticipationSpeedMultiplier, UAnimMontage* Montage);
	
	void EndAttackWarning();
	UFUNCTION(Server, Reliable)
	void Server_EndAttackWarning(UAnimMontage* Montage);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndAttackWarning(UAnimMontage* Montage);
	void PerformEndAttackWarning(UAnimMontage* Montage);
	const float AttackWarningRadius = 750.f;

	FORCEINLINE UAnimMontage* GetCurrentAttackMontage() const
	{
		return CurrentAttackMontage;
	}

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
	FTimerHandle AttackInterruptionTimer;
	float InterruptionDuration = 1.0f;
	
	/**
	 * The combo states that a montage ending doesn't reset like it would for other states.
	 */
	EComboState NonEndingComboStates = EComboState::Idle | EComboState::Broken;

public:
	virtual void AttackAction();
	FORCEINLINE void EnableRoll() { bCanRoll = true; }
	FORCEINLINE void DisableRoll() { bCanRoll = false; }
	// ROLLING
	virtual void Roll();
	virtual void RollRecover();
	FORCEINLINE void SetRollDuration(const float Duration) {RollDuration = Duration;}

	void OnStunned();
	void OnRecoverFromStun();

	void SetComboState(const EComboState State);
	FORCEINLINE EComboState GetComboState()  const {return ComboState;}

	bool bIsBroken = false;
};
