#pragma once

#include "CoreMinimal.h"
#include "Swordslike/UnitControllers/IEntityComponent.h"
#include "InputActionValue.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "TargetLockerComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLockStateChanged, bool);

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
	// Sets default values for this component's properties
	UTargetLockerComponent();
	// IEntityComponent
	virtual void InitEntityComponent(ACharacter* Character) override;
	
	// UFUNCTION()
	void PerformLockAction();

protected:
	// Called when the game starts.
	virtual void BeginPlay() override;

public:	
	// Called every frame.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULockWidgetController> LockIndicatorWidget;

	void SetLockIndicatorWidget(TObjectPtr<ULockWidgetController> LockIndicatorWidget);
	
	template<typename UserClass>
	void AddToOnLockedTarget(UserClass* Object, void (UserClass::*Function)(bool))
	{
		OnLockStateChanged.AddUObject(Object, Function);
	}
	
	FOnLockStateChanged OnLockStateChanged;

	UPROPERTY(ReplicatedUsing=OnRep_IsLockedOn)
	bool bIsLockedOnTarget = false;

	// server
	UFUNCTION(Server, Reliable)
	void Server_LockOn(ULockableTargetComponent* Target);
	UFUNCTION(Server, Reliable)
	void Server_Unlock();

	// client
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_LockOn(ULockableTargetComponent* Target);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Unlock();

private:
	float SearchRadius = 1500.0f;
	float DisconnectRadius = 2500.0f;

	bool bIsTimerRunning = false;
	bool bIsTimerFinished = false;
	FTimerHandle OutOfSightTimer;
	const float OffSightLockDuration = 3.f;

protected:
	UFUNCTION()
	void OnRep_IsLockedOn();

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

private:
	void LockOn();
	void Unlock();

	TObjectPtr<AController> CharacterController;
	TObjectPtr<UCameraComponent> Camera;
	TObjectPtr<USpringArmComponent> SpringArm;

	// TODO: Look into ways to avoid replicating this and use an RPC call instead.
	UPROPERTY(Replicated)
	TObjectPtr<ULockableTargetComponent> LockedTarget;
	TObjectPtr<UBaseHealthComponent> LockedTargetHealth;

	void OnUnlockedTarget();
	void OnLockedTarget();
	
	/**
	 * Returns a target that is within character view and within the lock distance.
	 */
	ULockableTargetComponent* FindTarget();

	// Tick functions
	void UpdateTargetLocation(float DeltaTime);
	/**
	 * Validates whether the lock should still take place depending on the target's state and location.
	 */
	void ValidateLock();
	/**
	 * 
	 * @return whether performing any lock-related logic is possible based on whether there's a lockable, the player character reference is valid, and more.
	 */

	
	bool CanPerformLock() const;
	bool IsTargetInRange(ULockableTargetComponent* Target) const;
};