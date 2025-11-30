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

class AHUDManager;
class ADestructibleObject;
class UPlayerStartCharacterDataAsset;
class UBaseEntityAnimationsComponent;
class UWeaponAttackIndicatorWidget;
class UOverheadHealthBarWidget;
enum class EParryState : uint8;
class UWeaponHandlerComponent;
class UTargetLockerComponent;
class UPlayerCombatComponent;
class UInteractionComponent;
class ULockWidgetController;
class UBaseCombatComponent;
class UInputMappingContext;
class UBaseParryComponent;
class USpringArmComponent;
struct FInputActionValue;
class ULegacyCameraShake;
class UNiagaraComponent;
class USphereComponent;
class USprintComponent;
class UWidgetComponent;
class UCameraComponent;
class UBaseEntityData;
class UNiagaraSystem;
class UInputAction;
class UMasterHUD;

DECLARE_LOG_CATEGORY_EXTERN(SwordslikeLog, Log, All);

DECLARE_MULTICAST_DELEGATE(DelegateEvent);

UCLASS(config=Game)
class ASwordslikeCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	ASwordslikeCharacter();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void Jump() override;
	virtual void Landed(const FHitResult& Hit) override;
	DelegateEvent OnJumped;
	DelegateEvent OnLanded;

	void Roll();
	void Attack();
	void Parry();
	void EndParry();
private:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerCharacterDataAsset)
	UPlayerStartCharacterDataAsset* PlayerCharacterDataAsset;
	UFUNCTION()
	void OnRep_PlayerCharacterDataAsset();
	void ApplyDataValuesToPlayer();
	void OnBeginPlay(APawn* Pawn);
	
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
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
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* WeaponDropPoint;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UCapsuleComponent* DestructibleCollider;
	APlayerController* PlayerController;
	UCapsuleComponent* Capsule;
	UMasterHUD* MasterHUD;
	AHUDManager* HUDManager;

	///////////////////////
	/// WIDGETS
	///////////////////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* OverheadHealthBar;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* WeaponAttackIndicatorWidgetComponent;
	UWeaponAttackIndicatorWidget* WeaponAttackIndicator;
	
	TObjectPtr<UOverheadHealthBarWidget> OverHeadHUD;
	TObjectPtr<UAnimInstance> AnimInstance;
	
	virtual void BeginPlay() override;

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);

	void Interact();
	
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void CacheComponentReferences();
	void InitializePlayerComponents();

	UPROPERTY(ReplicatedUsing=OnRep_PlayerName)
	FString PlayerName;
	
	UFUNCTION()
	void OnRep_PlayerName();
	void SetPlayerName(const FString& Name);
	UFUNCTION(Server, Reliable)
	void Server_SetPlayerName(const FString& Name);
	UFUNCTION(Client, Reliable)
	void Client_SetPlayerName();
	UFUNCTION(Server, Reliable)
	void Server_SetPlayerNameProcess(const FString& Name);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetPlayerName(const FString& Name);
	
	bool bIsInitialized = false;
	bool bOnBeginPlayerRegistered = false;
	void SetDefaultReplicationProperties();

public:
	FORCEINLINE UWidgetComponent* GetAttackIndicatorWidgetComponent() const {return WeaponAttackIndicatorWidgetComponent; }
	FORCEINLINE ULockableTargetComponent* GetLockableTargetComponent() const {return LockableTargetComponent; }
	FORCEINLINE UWeaponAttackIndicatorWidget* GetAttackIndicatorWidget() const {return WeaponAttackIndicator; }
	FORCEINLINE UCapsuleComponent* GetDestructibleSphereComponent() const {return DestructibleCollider; }
	FORCEINLINE UInteractionComponent* GetInteractionComponent() const {return InteractionComponent; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE UPlayerStartCharacterDataAsset* GetData() const { return PlayerCharacterDataAsset; }
	FORCEINLINE UTargetLockerComponent* GetLockOnComponent() const {return TargetLockerComponent; }
	FORCEINLINE UOverheadHealthBarWidget* GetOverHeadHUDComponent() const {return OverHeadHUD; }
	FORCEINLINE ULockWidgetController* GetLockOnWidget() const {return LockIndicatorWidget; }
	FORCEINLINE UWeaponHandlerComponent* GetWeaponHandler() const {return WeaponHandler; }
	FORCEINLINE UBaseEntityAnimationsComponent* GetAnimation() const {return Animations; }
	FORCEINLINE APlayerController* GetPlayerController() const {return PlayerController; }
	FORCEINLINE UBaseParryComponent* GetParryComponent() const {return ParryComponent; }
	FORCEINLINE USceneComponent* GetWeaponDropPoint() const {return WeaponDropPoint; }
	FORCEINLINE UPlayerCombatComponent* GetCombatComponent() const {return Combat; }
	FORCEINLINE UPlayerHealthComponent* GetHealthComponent() const {return Health; }
	FORCEINLINE USkeletalMeshComponent* GetCustomMesh() const {return CustomMesh; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE UCapsuleComponent* GetInteractionSphere() const {return Capsule; }
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UAnimInstance* GetAnimInstance() const {return AnimInstance; }
	FORCEINLINE USprintComponent* GetSprintComponent() const {return Sprint; }
	FORCEINLINE UMasterHUD* GetMasterHUD() const {return MasterHUD; }
	
	FORCEINLINE void SetCanAttack(const bool NewCanAttack) {bCanAttack = NewCanAttack; }
	FORCEINLINE void SetCanMove(const bool NewCanMove) {bCanMove = NewCanMove; }
	FORCEINLINE void SetCanJump(const bool NewCanJump) {bCanJump = NewCanJump; }
	FORCEINLINE bool GetCanAttack() const {return bCanAttack;}

	UPROPERTY(ReplicatedUsing=OnRep_CurrentSpeed)
	float CurrentSpeed;
	UFUNCTION()
	void OnRep_CurrentSpeed();
	void OnDeath();
	UFUNCTION(Client, Reliable)
	void Client_OnDeath();
	void SetSprintSpeed();
	void ResetSpeed();

	FORCEINLINE FVector2D GetMovementVector() const { return MovementVector; }
	bool bIsDebugging = false;

private:
	FVector2D MovementVector;

	void SetInitialValues();
	UFUNCTION(Server, Reliable)
	void Server_SetInitialValues(UPlayerStartCharacterDataAsset* Data);
	void PerformSetInitialValues(UPlayerStartCharacterDataAsset* Data);

	/**
	 * Called when the entity takes damage
	 */
	FTimerHandle HitRecoveryTimer;
	const float RecoveryDuration = 0.5f;

	bool bCanJump = true;
	bool bCanMove = true;
	bool bCanAttack = true;
	
	UFUNCTION(Server, Reliable)
	void Server_SetWalkSpeed(float NewSpeed);

	// TEST
	void PerformTestActoin();
	FTimerHandle TestTimeHandle;

	UFUNCTION()
	void OnDestructibleOverlapped(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(Server, Reliable)
	void Server_InflictDamageToDestructible(ADestructibleObject* Destructible, FDamageInfo DamageInfo);

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
	void OnCharacterHitProcess(const FDamageInfo& DamageInfo);
	UFUNCTION(Client, Reliable)
	void Client_OnCharacterHit(const FDamageInfo& DamageInfo);
	UFUNCTION(Server, Reliable)
	void Server_PerformDamagePostureOnAttacker(UBaseParryComponent* AttackerParry, const FDamageInfo& DamageInfo);

	void CustomLaunchCharacter(ASwordslikeCharacter* Character, const FVector LaunchVector);
	UFUNCTION(Server, Reliable)
	void Server_CustomLaunchCharacter(ASwordslikeCharacter* PushedCharacter, const FVector LaunchVector);
	void PerformCustomLauchCharacter(ASwordslikeCharacter* Character, const FVector& HitLocation);
	
	void OnCharacterHitRecovered();
	void OnRollStarted();
	void OnRollFinished();
	void OnSprintStarted();
	void OnSprintEnded();

	void RotateCharacterToDirection(const FRotator& NewRotation);
	void SetOverheadText(const FString& Message);

	void PerformCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass);
	UFUNCTION(Client, Reliable)
	void Client_PlayCameraShake(TSubclassOf<UCameraShakeBase> ShakeClass);
	void CameraShakeProcess(TSubclassOf<UCameraShakeBase> ShakeClass);
	

	void EnableDestructibleCollider();
	void DisableDestructibleCollider();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* ParrySparkVFX;

public:
	FString GetInteractionInput();

private:
	FString GetInputKey(const UInputAction* InputAction);

public:
	UPROPERTY(EditDefaultsOnly, Category="Settings")
	TSubclassOf<UCameraShakeBase> CameraShake;
	
	UPROPERTY(EditDefaultsOnly, Category=CameraShake)
	TSubclassOf<UCameraShakeBase> HitCameraShake;
	UPROPERTY(EditDefaultsOnly, Category=CameraShake)
	TSubclassOf<UCameraShakeBase> NormalParryCameraShake;
	UPROPERTY(EditDefaultsOnly, Category=CameraShake)
	TSubclassOf<UCameraShakeBase> GoodParryCameraShake;
	UPROPERTY(EditDefaultsOnly, Category=CameraShake)
	TSubclassOf<UCameraShakeBase> PerfectParryCameraShake;

private:
	const float ParryPushBackForce = 500.0f;
	bool CanJump() const;
};

