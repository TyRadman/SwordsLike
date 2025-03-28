// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Swordslike/Combat/Interactable.h"
#include "Weapon.generated.h"

class UNiagaraComponent;
class USphereComponent;
class AWeaponInteractable;
class UArrowComponent;

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	
	EWS_MAX UMETA(DisplayName = "DefaultMAX"),
};

UCLASS()
class SWORDSLIKE_API AWeapon : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	AWeapon();

protected:
	virtual void BeginPlay() override;

public:
	void OnWeaponEquipped();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FString WeaponName{TEXT("WeaponName")};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UArrowComponent* StartArrow;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UArrowComponent* EndArrow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamagePerHit;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float PostureDamagePerHit = 5.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StaminaPerHit;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float HitBoxRadius;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector LocationOffset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRotator RotationOffset;

	/**
	 * Determines whether the character should extend its army when moving around with the weapon. Mere visuals.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Visuals)
	bool bIsCarryingHeavyWeapon;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Visuals)
	UAnimMontage* ComboMontage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Visuals)
	TArray<UAnimMontage*> ComboMontages;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Visuals)
	UParticleSystem* ImpactParticles;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(VisibleAnywhere, Category="Components")
	USphereComponent* AreaSphere;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Components", meta=(AllowPrivateAccess = true))
	UNiagaraComponent* TrailEffect;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Effects", meta=(AllowPrivateAccess = true))
	UParticleSystemComponent* TrailPSC;


	UPROPERTY(VisibleAnywhere)
	EWeaponState WeaponState;

	bool bIsEquipped = false;

public:
	virtual void Interact(AActor* Interactor) override;
	UFUNCTION(Server, Reliable)
	void Server_Interact(AActor* InteractingActor);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Interact(AActor* InteractingActor);

	void InteractionProcess(AActor* InteractingActor);
	
	virtual FString GetInteractionMessage() override;

	FORCEINLINE UStaticMeshComponent* GetMesh() const { return Mesh; }
	FORCEINLINE UParticleSystemComponent* GetTrailEffect() const { return TrailPSC; }
	

};
