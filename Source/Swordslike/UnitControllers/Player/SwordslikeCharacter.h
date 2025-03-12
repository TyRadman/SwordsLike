// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Swordslike/UnitControllers/Player/LockWidgetController.h"
#include "Swordslike/UnitControllers/Player/TargetLockerComponent.h"
#include "Swordslike/UnitControllers/Player/PlayerHealthComponent.h"

#include "SwordslikeCharacter.generated.h"

class UBaseParryComponent;
class USprintComponent;
class UWidgetComponent;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class ULockWidgetController;
class UTargetLockerComponent;
class UPlayerCombatComponent;
class UBaseEntityAnimationsComponent;
class UBaseEntityData;
class UWeaponHandlerComponent;
class UNiagaraComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

DECLARE_MULTICAST_DELEGATE(DelegateEvent);

UCLASS(config=Game)
class ASwordslikeCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	ASwordslikeCharacter();
	
	virtual void Jump() override;
	virtual void Landed(const FHitResult& Hit) override;
	DelegateEvent OnJumped;
	DelegateEvent OnLanded;

	void Roll();
	void Attack();
	void Parry();
	void EndParry();
private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Stats, meta=(AllowPrivateAccess = "true"))
	UBaseEntityData* PlayerStats;

	virtual void Tick(float DeltaTime) override;
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LockAction;
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* SprintAction;
	UPROPERTY(EditDefaultsOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> AttackInputAction;
	UPROPERTY(EditDefaultsOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> RollInputAction;
	UPROPERTY(EditDefaultsOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> TestInputAction;
	UPROPERTY(EditDefaultsOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ParryInputAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	UPlayerHealthComponent* Health;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations", meta = (AllowPrivateAccess = "true"))
	UBaseEntityAnimationsComponent* Animations;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = LockOn, meta = (AllowPrivateAccess = "true"))
	ULockWidgetController* LockIndicatorWidget;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "LockOn")
	UTargetLockerComponent* TargetLockerComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UPlayerCombatComponent* Combat;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USprintComponent* Sprint;

	ULockableTargetComponent* LockableTargetComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Visuals, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* CustomMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UWeaponHandlerComponent* WeaponHandler;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UBaseParryComponent* ParryComponent;

	///////////////////////
	/// WIDGETS
	//////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* DebuggerText;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadHealthBar;
	
	virtual void BeginPlay() override;

protected:
	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UWeaponHandlerComponent* GetWeaponHandler() const {return WeaponHandler;}
	FORCEINLINE UPlayerHealthComponent* GetHealthComponent() const {return Health;}
	FORCEINLINE UBaseEntityData* GetPlayerStats() const {return PlayerStats;}
	FORCEINLINE UPlayerCombatComponent* GetCombat() const {return Combat;}
	FORCEINLINE UTargetLockerComponent* GetLockOnComponent() const {return TargetLockerComponent;}
	FORCEINLINE ULockWidgetController* GetLockOnWidget() const {return LockIndicatorWidget;}
	FORCEINLINE USprintComponent* GetSprintComponent() const {return Sprint;}
	FORCEINLINE USkeletalMeshComponent* GetCustomMesh() const {return CustomMesh;}
	FORCEINLINE UBaseParryComponent* GetParryComponent() const {return ParryComponent;}

	void OnDeath(const FDamageInfo& DamageInfo);
	void SetSprintSpeed();
	void ResetSpeed();

	FVector2D GetMovementVector() const;

private:
	FVector2D MovementVector;
	
	void OnTargetLockedOn(bool IsLockedOn);

	void SetInitialValues();

	/**
	 * Called when the entity takes damage
	 */
	void OnCharacterHit(const FDamageInfo& DamageInfo);
	void OnCharacterHitRecovered();
	void OnRollStarted();
	void OnRollFinished();
	void OnParryStarted();
	void OnParryEnded();
	
	FTimerHandle HitRecoveryTimer;
	const float RecoveryDuration = .5f;

	bool bCanJump = true;
	bool bCanMove = true;
	
	UFUNCTION(Server, Reliable)
	void Server_SetWalkSpeed(float NewSpeed);

	// TEST
	void StartAttackCycle();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* ParrySparkVFX;

};

