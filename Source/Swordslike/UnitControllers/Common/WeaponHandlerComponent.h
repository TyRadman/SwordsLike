#pragma once

#include "CoreMinimal.h"
#include "Damagable.h"
#include "SprintComponent.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "Weapons/Weapon.h"
#include "WeaponHandlerComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(WeaponHitDelegate, AWeapon* Weapon);

class UArrowComponent;
class ASwordslikeCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API UWeaponHandlerComponent : public UMyActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponHandlerComponent();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	WeaponHitDelegate OnWeaponHitStarted;
	void StartWeaponAttackDetection();
	void StopWeaponAttackDetection();
	void Setup(ASwordslikeCharacter* Character);


	void EquipWeapon(AWeapon* Weapon);
	float GetWeaponStaminaCost() const;
	FVector GetWeaponMiddleLocation() const;

	FORCEINLINE AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

private:
	bool bIsAttacking = false;

	void GetTargetsInWeaponRange();
	
	TArray<IDamagable*> TargetsHit;

	ASwordslikeCharacter* WeaponOwner;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentWeapon)
	AWeapon* CurrentWeapon;
	
	UFUNCTION()
	void OnRep_CurrentWeapon();

	UFUNCTION(Server, Reliable)
	void Server_SpawnDefaultWeapon(TSubclassOf<AWeapon> WeaponClass);
	
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AWeapon* Weapon);
	void EquipWeaponProcess(AWeapon* Weapon);
	
	AWeapon* DefaultWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	TSubclassOf<AWeapon> StartingWeapon;

	FTimerHandle AttackTimer;

	FVector PreviousStartLocation;
	FVector PreviousEndLocation;

	void GetTargetsFromHitResults(TArray<FHitResult>& HitResults);
	void CacheTargetsBetweenTwoPoints(const FVector& StartLocation, const FVector& EndLocation);
};
