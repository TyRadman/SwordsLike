#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Swordslike/UnitControllers/Player/LockWidgetController.h"
#include "Swordslike/UnitControllers/Player/TargetLockerComponent.h"
#include "Swordslike/UnitControllers/Player/PlayerHealthComponent.h"

#include "SwordslikeCharacter.generated.h"

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
	
	UOverheadHealthBarWidget* OverHeadHUD;
	UAnimInstance* AnimInstance;
	
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Interact();
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void CacheComponentReferences();
	void InitializeComponents();

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UWeaponHandlerComponent* GetWeaponHandler() const {return WeaponHandler;}
	FORCEINLINE UPlayerHealthComponent* GetHealthComponent() const {return Health;}
	FORCEINLINE UBaseEntityData* GetPlayerStats() const {return PlayerStats;}
	FORCEINLINE UPlayerCombatComponent* GetCombatComponent() const {return Combat;}
	FORCEINLINE UTargetLockerComponent* GetLockOnComponent() const {return TargetLockerComponent;}
	FORCEINLINE ULockWidgetController* GetLockOnWidget() const {return LockIndicatorWidget;}
	FORCEINLINE USprintComponent* GetSprintComponent() const {return Sprint;}
	FORCEINLINE USkeletalMeshComponent* GetCustomMesh() const {return CustomMesh;}
	FORCEINLINE UBaseParryComponent* GetParryComponent() const {return ParryComponent;}
	FORCEINLINE UOverheadHealthBarWidget* GetOverHeadHUDComponent() const {return OverHeadHUD;}
	FORCEINLINE UInteractionComponent* GetOverInteractionComponent() const {return InteractionComponent;}
	FORCEINLINE UAnimInstance* GetAnimInstance() const {return AnimInstance;}
	FORCEINLINE UCapsuleComponent* GetInteractionSphere() const {return Capsule;}
	FORCEINLINE UMasterHUD* GetMasterHUD() const {return MasterHUD;}
	FORCEINLINE UBaseEntityAnimationsComponent* GetAnimation() const {return Animations;}
	FORCEINLINE ULockableTargetComponent* GetLockableTargetComponent() const {return LockableTargetComponent;}
	

	void OnDeath(const FDamageInfo& DamageInfo);
	void SetSprintSpeed();
	void ResetSpeed();

	FORCEINLINE FVector2D GetMovementVector() const { return MovementVector; }

private:
	FVector2D MovementVector;

	void SetInitialValues();

	/**
	 * Called when the entity takes damage
	 */
	
	FTimerHandle HitRecoveryTimer;
	const float RecoveryDuration = .5f;

	bool bCanJump = true;
	bool bCanMove = true;
	
	UFUNCTION(Server, Reliable)
	void Server_SetWalkSpeed(float NewSpeed);

	// TEST
	void StartAttackCycle();

	FORCEINLINE void SetCanMove(bool NewCanMove) {bCanMove = NewCanMove; }
	FORCEINLINE void SetCanJump(bool NewCanJump) {bCanJump = NewCanJump; }

public:
	void OnKnockedDown();
	void OnKnockedDownRecover();
	void OnAttackParried(const FDamageInfo& DamageInfo, EParryState State);
	void OnTargetLockedOn(bool IsLockedOn);
	void OnCharacterHit(const FDamageInfo& DamageInfo);
	void OnCharacterHitRecovered();
	void OnRollStarted();
	void OnRollFinished();
	void OnSprintStarted();
	void OnSprintEnded();

private:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* ParrySparkVFX;

public:
	FString GetInteractionInput();
	FString PlayerName;

private:
	FString GetInputKey(UInputAction* InputAction);
};

