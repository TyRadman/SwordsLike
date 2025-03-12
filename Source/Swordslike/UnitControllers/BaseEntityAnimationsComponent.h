// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "BaseEntityAnimationsComponent.generated.h"

class ASwordslikeCharacter;
class UAnimMontage;
class UAnimInstance;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API UBaseEntityAnimationsComponent : public UMyActorComponent, public IIEntityComponent
{
	GENERATED_BODY()

public:	
	UBaseEntityAnimationsComponent();

	virtual void InitEntityComponent(ACharacter* Character) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	float GetRollAnimationDuration() const;

protected:
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* HitReactMontage;

	UFUNCTION(BlueprintCallable)
	void PlayHitReactMontage();

	// ROLL
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* RollMontage_F;
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* RollMontage_B;
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* RollMontage_L;
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* RollMontage_R;
	
	UAnimMontage* GetRollMontage();

	void PlayRollMontage();

	UFUNCTION(Server, Reliable)
	void Server_PlayRollMontage(UAnimMontage* Montage);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayRollMontage(UAnimMontage* Montage);

	

private:
	UPROPERTY(VisibleDefaultsOnly, Category=Animations)
	UAnimInstance* AnimInstance;

	// utilities
	void PlayMontage(UAnimMontage* Montage);

	ASwordslikeCharacter* OwnerCharacter;
};
