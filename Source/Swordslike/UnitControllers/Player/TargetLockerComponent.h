#pragma once

#include "CoreMinimal.h"
#include "Swordslike/UnitControllers/IEntityComponent.h"
#include "Swordslike/UnitControllers/Common/LockableTargetComponent.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "TargetLockerComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLockStateChanged, ULockableTargetComponent* Target, bool bIsLockedOn);

class UCameraComponent;
class UBaseHealthComponent;
class ULockableTargetComponent;
class UInputMappingContext;
class UInputAction;
class ULockWidgetController;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SWORDSLIKE_API UTargetLockerComponent : public UMyActorComponent, public IIEntityComponent
{
	GENERATED_BODY()

public:	
	UTargetLockerComponent();
	virtual void InitEntityComponent(ACharacter* Character) override;
	void PerformLockAction();
	

	UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULockWidgetController> LockIndicatorWidget;

	void SetLockIndicatorWidget(const TObjectPtr<ULockWidgetController>& LockIndicatorWidgetReference) {LockIndicatorWidget = LockIndicatorWidgetReference;}
	template<typename UserClass>
	FORCEINLINE void AddToOnLockedTarget(UserClass* Object, void (UserClass::*Function)(ULockableTargetComponent, bool)){Server_OnLockStateChanged.AddUObject(Object, Function);}
	FORCEINLINE bool GetIsLocked() const {return bIsLockedOnTarget;}
	
	FOnLockStateChanged Server_OnLockStateChanged;
	FOnLockStateChanged Local_OnLockStateChanged;


private:
	const float SearchRadius = 2500.0f;
	const float DisconnectRadius = 3000.0f;
	const float OffSightLockDuration = 3.f;

	bool bIsTimerRunning = false;
	bool bIsTimerFinished = false;
	FTimerHandle OutOfSightTimer;

	UPROPERTY(ReplicatedUsing=OnRep_bIsLockedOnTarget)
	bool bIsLockedOnTarget = false;
	UFUNCTION()
	void OnRep_bIsLockedOnTarget();

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(Server, Reliable)
	void Server_LockOn(ULockableTargetComponent* Target);
	UFUNCTION(Server, Reliable)
	void Server_Unlock();
	
	// TODO: Look into ways to avoid replicating this and use an RPC call instead.
	UPROPERTY(Replicated)
	ULockableTargetComponent* LockedTarget;

private:
	void LockOn();
	void Unlock();

	TObjectPtr<AController> CharacterController;
	TObjectPtr<UCameraComponent> Camera;
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(Replicated)
	TObjectPtr<UBaseHealthComponent> LockedTargetHealth;

	void OnUnlockedTarget();
	void OnLockedTarget(ULockableTargetComponent* Target);
	
	/**
	 * Returns a target that is within character view and within the lock distance.
	 */
	ULockableTargetComponent* FindTarget();

	// Tick functions
	void UpdateTargetLocation(const float DeltaTime);

	/**
	 * Validates whether the lock should still take place depending on the target's state and location.
	 */
	void ValidateLock();
	
	/**
	 * 
	 * @return whether performing any lock-related logic is possible based on whether there's a lockable, the player character reference is valid, and more.
	 */
	bool CanPerformLock() const;
	bool IsTargetInRange(const ULockableTargetComponent* Target) const;
};