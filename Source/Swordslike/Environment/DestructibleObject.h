#pragma once

#include "CoreMinimal.h"
#include "Common/Damagable.h"
#include "GameFramework/Actor.h"
#include "DestructibleObject.generated.h"

class UArrowComponent;
class UGeometryCollection;

/**
 * A base class for objects that can be destroyed in the environment.
 */
UCLASS()
class SWORDSLIKE_API ADestructibleObject : public AActor, public IDamagable
{
	GENERATED_BODY()
	
public:
	ADestructibleObject();

protected:
	virtual void BeginPlay() override;
	
	virtual bool IsAlive() override {return true;}
	virtual bool IsInvincible() const override {return false;}

	UPROPERTY(EditDefaultsOnly, Category=References, meta=(AllowPrivateAccess=true))
	UGeometryCollectionComponent* GeometryCollectionComponent;
	UPROPERTY(EditDefaultsOnly, Category=References, meta=(AllowPrivateAccess=true))
	UStaticMeshComponent* DestructibleMesh;
	UPROPERTY(EditDefaultsOnly, Category=References, meta=(AllowPrivateAccess=true))
	UArrowComponent* ImpulsePoint;
	
	UPROPERTY(EditDefaultsOnly, Category=References, meta=(AllowPrivateAccess=true))
	float ImpulseForce = 2500.f;
	UPROPERTY(EditDefaultsOnly, Category=References, meta=(AllowPrivateAccess=true))
	float ImpulseRadius;

	FTimerHandle FadeTimerHandle;
	float CurrentFade = 1.0f;
	void FadeOutDebris();
	TArray<UMaterialInstanceDynamic*> DynamicMaterials;

	bool bIsDestroyed = false;


public:	
	virtual void TakeDamage(const FDamageInfo& DamageInfo) override;

	UFUNCTION(Server, Reliable)
	void Server_SetupColliders(const FDamageInfo& DamageInfo);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetupColliders(const FDamageInfo& DamageInfo);
	void PerformSetupColliders(const FDamageInfo& DamageInfo);
	void ActivateGeometryCollection(const FDamageInfo& DamageInfo);

};
