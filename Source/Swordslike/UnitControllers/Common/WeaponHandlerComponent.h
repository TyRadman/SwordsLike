#pragma once

#include "CoreMinimal.h"
#include "Damagable.h"
#include "SprintComponent.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "Weapons/Weapon.h"
#include "WeaponHandlerComponent.generated.h"

enum class EHitType : uint8;
DECLARE_MULTICAST_DELEGATE_OneParam(WeaponHitDelegate, AWeapon* Weapon);

class UArrowComponent;
class ASwordslikeCharacter;
class UCameraShakeBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API UWeaponHandlerComponent : public UMyActorComponent, public IIEntityComponent
{
	GENERATED_BODY()

public:	
	UWeaponHandlerComponent();

protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	virtual void InitEntityComponent(ACharacter* Character) override;
	
	UAnimMontage* GetAttackMontage() const;
	UAnimMontage* GetNextAttackMontage();
	void ResetAttackMontages();
	const UAnimMontage* GetCurrentAttackMontage();
	int32 CurrentAttackAnimationIndex = 0;
	FORCEINLINE bool HasWeapon() const { return CurrentWeapon != nullptr; ;}

	WeaponHitDelegate OnWeaponHitStarted;
	void StartWeaponAttackDetection(const EHitType NewHitType, const float NewDamage, const TSubclassOf<UCameraShakeBase>& CameraShake);
	void StopWeaponAttackDetection();
	
	UFUNCTION(Server, Reliable)
	void Server_OnTargetAttacked(const FVector ImpactPoint, AWeapon* Weapon);
	UFUNCTION(NetMulticast, Reliable)
	void Multicasat_OnTargetAttacked(const FVector ImpactPoint, AWeapon* Weapon);


	void EquipWeapon(AWeapon* WeaponToEquip);
	float GetWeaponStaminaCost() const;
	FVector GetWeaponMiddleLocation() const;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentWeapon)
	AWeapon* CurrentWeapon;

	void DropWeapon();

	ASwordslikeCharacter* PlayerCharacter;
	bool bIsLocallyController = false;

private:
	bool bIsAttacking = false;
	EHitType CurrentHitType;
	float CurrentDamage;
	TSubclassOf<UCameraShakeBase> CurrentCameraShake;

	void GetTargetsInWeaponRange();
	
	TArray<IDamagable*> TargetsHit;

	ASwordslikeCharacter* WeaponOwner;
	
	UFUNCTION()
	void OnRep_CurrentWeapon();
	
	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon(AWeapon* WeaponToEquip);
	void EquipWeaponProcess(AWeapon* WeaponToEquip);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess=true))
	TSubclassOf<AWeapon> StartingWeapon;

	FTimerHandle AttackTimer;

	FVector PreviousStartLocation;
	FVector PreviousEndLocation;

	void GetTargetsFromHitResults(TArray<FHitResult>& HitResults);
	void CheckForDetectedCharacter(const FHitResult& HitResult, AActor* Target);
	void CheckForDetectedNonCharacter(const FHitResult& HitResult, AActor* Target);
	void CacheTargetsBetweenTwoPoints(const FVector& StartLocation, const FVector& EndLocation);

	// equip animations
	UAnimInstance* AnimInstance;
	
	UPROPERTY(EditDefaultsOnly, Category=Animations)
	UAnimMontage* EquipMontage;
	
	void PlayEquipMontage();

	void PlayMontage(UAnimMontage* Montage);
	UFUNCTION(Server, Reliable)
	void Server_PlayMontage(UAnimMontage* Montage);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(UAnimMontage* Montage);
	
	UFUNCTION(Server, Reliable)
	void Server_InflictDamageOnComponent(UActorComponent* TargetComponent, const FDamageInfo& DamageInfo);
	UFUNCTION(Server, Reliable)
	void Server_InflictDamageOnActor(AActor* Target, const FDamageInfo& DamageInfo);


public:
	UPROPERTY(Replicated, BlueprintReadOnly)
	bool bIsCarryingHeavyWeapon = false;
	FORCEINLINE AWeapon* GetCurrentWeapon() const { return CurrentWeapon; }

	void OnForceStopAttack();
};
