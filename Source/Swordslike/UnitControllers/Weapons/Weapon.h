// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

class UArrowComponent;

UCLASS()
class SWORDSLIKE_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UArrowComponent* StartArrow;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UArrowComponent* EndArrow;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DamagePerHit;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float StaminaPerHit;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float HitBoxRadius;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FVector LocationOffset;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FRotator RotationOffset;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = true))
	UStaticMeshComponent* Mesh;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	USceneComponent* SceneRoot;

};
