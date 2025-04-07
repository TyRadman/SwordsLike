#pragma once

#include "CoreMinimal.h"
#include "PlayerCombatComponent.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Swordslike/UnitControllers/Player/LockWidgetController.h"
#include "Swordslike/UnitControllers/Player/TargetLockerComponent.h"
#include "Swordslike/UnitControllers/Player/PlayerHealthComponent.h"
#include "NiagaraSystem.h"
#include "Camera/CameraShakeBase.h"
#include "SwordslikeCharacter.generated.h"

class UPlayerStartCharacterDataAsset;
class UNiagaraSystem;
class UWeaponAttackIndicatorWidget;
class UBaseCombatComponent;
class UMasterHUD;
class USphereComponent;
class UInteractionComponent;
enum class EParryState : uint8;
class UOverheadHealthBarWidget;
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
class ULegacyCameraShake;

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
	// UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Stats, meta=(AllowPrivateAccess = "true"))
	// UBaseEntityData* PlayerStats;
	UPROPERTY(ReplicatedUsing = OnRep_PlayerCharacterDataAsset)
	UPlayerStartCharacterDataAsset* PlayerCharacterDataAsset;
	UFUNCTION()
	void OnRep_PlayerCharacterDataAsset();
	void ApplyDataValuesToPlayer();
	
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_PlayerState() override;
	
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
	UPROPERTY(EditDefaultsOnly, Category= Input, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> InteractActionInput;
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ULockableTargetComponent* LockableTargetComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Visuals, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* CustomMesh;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UWeaponHandlerComponent* WeaponHandler;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UBaseParryComponent* ParryComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UInteractionComponent* InteractionComponent;
	
	UCapsuleComponent* Capsule;
	UMasterHUD* MasterHUD;

	///////////////////////
	/// WIDGETS
	///////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadHealthBar;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* WeaponAttackIndicatorWidgetComponent;
	UWeaponAttackIndicatorWidget* WeaponAttackIndicator;
	
	UOverheadHealthBarWidget* OverHeadHUD;
	UAnimInstance* AnimInstance;
	
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Interact();
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void CacheComponentReferences();
	void InitializeComponents();
	void SetDefaultReplicationProperties();

public:
	FORCEINLINE UWidgetComponent* GetAttackIndicatorWidgetComponent() const {return WeaponAttackIndicatorWidgetComponent; }
	FORCEINLINE ULockableTargetComponent* GetLockableTargetComponent() const {return LockableTargetComponent; }
	FORCEINLINE UWeaponAttackIndicatorWidget* GetAttackIndicatorWidget() const {return WeaponAttackIndicator; }
	FORCEINLINE UInteractionComponent* GetOverInteractionComponent() const {return InteractionComponent; }
	FORCEINLINE UTargetLockerComponent* GetLockOnComponent() const {return TargetLockerComponent; }
	FORCEINLINE UOverheadHealthBarWidget* GetOverHeadHUDComponent() const {return OverHeadHUD; }
	FORCEINLINE ULockWidgetController* GetLockOnWidget() const {return LockIndicatorWidget; }
	FORCEINLINE UWeaponHandlerComponent* GetWeaponHandler() const {return WeaponHandler; }
	FORCEINLINE UBaseEntityAnimationsComponent* GetAnimation() const {return Animations; }
	FORCEINLINE UBaseParryComponent* GetParryComponent() const {return ParryComponent; }
	FORCEINLINE UPlayerCombatComponent* GetCombatComponent() const {return Combat; }
	FORCEINLINE UPlayerHealthComponent* GetHealthComponent() const {return Health; }
	FORCEINLINE USkeletalMeshComponent* GetCustomMesh() const {return CustomMesh; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UCapsuleComponent* GetInteractionSphere() const {return Capsule; }
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE USprintComponent* GetSprintComponent() const {return Sprint; }
	FORCEINLINE UAnimInstance* GetAnimInstance() const {return AnimInstance; }
	FORCEINLINE UMasterHUD* GetMasterHUD() const {return MasterHUD; }
	
	FORCEINLINE void SetCanAttack(const bool NewCanAttack) {bCanAttack = NewCanAttack; }
	FORCEINLINE void SetCanMove(const bool NewCanMove) {bCanMove = NewCanMove; }
	FORCEINLINE void SetCanJump(const bool NewCanJump) {bCanJump = NewCanJump; }

	UPROPERTY(ReplicatedUsing=OnRep_CurrentSpeed)
	float CurrentSpeed;
	UFUNCTION()
	void OnRep_CurrentSpeed();
	void OnDeath();
	void SetSprintSpeed();
	void ResetSpeed();

	FORCEINLINE FVector2D GetMovementVector() const { return MovementVector; }

private:
	FVector2D MovementVector;

	void SetInitialValues();
	UFUNCTION(Server, Reliable)
	void Server_SetInitialValues(UPlayerStartCharacterDataAsset* Data);
	// UFUNCTION(NetMulticast, Reliable)
	// void Multicast_SetInitialValues(UPlayerStartCharacterDataAsset* Data);
	void PerformSetInitialValues(UPlayerStartCharacterDataAsset* Data);

	/**
	 * Called when the entity takes damage
	 */
	FTimerHandle HitRecoveryTimer;
	const float RecoveryDuration = .5f;

	bool bCanJump = true;
	bool bCanMove = true;
	bool bCanAttack = true;
	
	UFUNCTION(Server, Reliable)
	void Server_SetWalkSpeed(float NewSpeed);

	// TEST
	void StartAttackCycle();

public:
	/**
	 * Restores the character and character controller rotation and orientation to its state based on the target locking state, whether the character is sprinting or not and more.
	 */
	void RestoreCharacterRotation();
	void OnStunned();
	void OnStunnedRecover();
	
	void OnAttackParried(EParryState ParryState);

	UPROPERTY(EditDefaultsOnly, Category=Parry)
	UNiagaraSystem* NormalParryParticle;
	UPROPERTY(EditDefaultsOnly, Category=Parry)
	UNiagaraSystem* GoodParryParticle;
	UPROPERTY(EditDefaultsOnly, Category=Parry)
	UNiagaraSystem* PerfectParryParticle;
	TMap<EParryState, UNiagaraSystem*> ParryVFXMap;

	
	void OnTargetLockedOn(ULockableTargetComponent* Target, bool bIsLockedOn);
	void SetLockOnValue(const bool NewIsLockedOn);
	UFUNCTION(Server, Reliable)
	void Server_OnTargetLockedOn(bool bIsLockedOn);
	void HandleOnTargetLockedOn(ULockableTargetComponent* Target, const bool bIsLockedOn);
	UPROPERTY(ReplicatedUsing = OnRep_bIsLockedOnTarget)
	bool bIsLockedOnTarget = false;
	UFUNCTION()
	void OnRep_bIsLockedOnTarget();
	
	void OnCharacterHit(const FDamageInfo& DamageInfo);
	UFUNCTION(Server, Reliable)
	void Server_OnCharacterHit(const FDamageInfo& DamageInfo);
	void PerformOnCharacterHit(const FDamageInfo& DamageInfo);
	
	void OnCharacterHitRecovered();
	void OnRollStarted();
	void OnRollFinished();
	void OnSprintStarted();
	void OnSprintEnded();

	void RotateCharacterToDirection(const FRotator& NewRotation);
	void PrintOverhead(const FString& Message);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* ParrySparkVFX;

public:
	FString GetInteractionInput();
	FString PlayerName;

private:
	FString GetInputKey(const UInputAction* InputAction);

public:
	UPROPERTY(EditDefaultsOnly, Category="Settings")
	TSubclassOf<UCameraShakeBase> CameraShake;
};

