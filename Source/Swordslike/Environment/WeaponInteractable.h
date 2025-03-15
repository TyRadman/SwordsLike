// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Swordslike/Combat/Interactable.h"
#include "WeaponInteractable.generated.h"

class USphereComponent;
class AWeapon;

UCLASS()
class SWORDSLIKE_API AWeaponInteractable : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	AWeaponInteractable();

	virtual void Interact(AActor* InteractingActor) override;
	virtual FString GetInteractionMessage() override;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Interact(AActor* InteractingActor);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Interact(AActor* InteractingActor);

	void InteractionProcess(AActor* InteractingActor);

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	TSubclassOf<AWeapon> Weapon;

	UPROPERTY(Replicated)
	AWeapon* WeaponInstance;

private:
	USphereComponent* Collider;

};
