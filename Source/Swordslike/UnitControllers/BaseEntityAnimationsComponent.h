// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "BaseEntityAnimationsComponent.generated.h"

struct FDamageInfo;
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
	FORCEINLINE float GetRollAnimationDuration() const {return RollMontage_F->GetPlayLength();}

	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* HitReactMontage;
	
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* ForwardHitReactMontage;
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* BackwardHitReactMontage;
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* LeftHitReactMontage;
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* RightHitReactMontage;

	// 1. normal hit 2. big hit 3. knock out
	UPROPERTY(EditAnywhere, Category=Animations)
	TArray<UAnimMontage*> ForwardHitReactMontages;
	UPROPERTY(EditAnywhere, Category=Animations)
	TArray<UAnimMontage*> BackwardHitReactMontages;
	UPROPERTY(EditAnywhere, Category=Animations)
	TArray<UAnimMontage*> LeftHitReactMontages;
	UPROPERTY(EditAnywhere, Category=Animations)
	TArray<UAnimMontage*> RightHitReactMontages;

	UFUNCTION(BlueprintCallable)
	void PlayHitReactMontage(const FDamageInfo& DamageInfo);

	// ROLL
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* RollMontage_F;
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* RollMontage_B;
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* RollMontage_L;
	UPROPERTY(EditAnywhere, Category=Animations)
	UAnimMontage* RollMontage_R;
	
	UAnimMontage* GetRollMontage() const;

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
	void PerformPlayMontage(UAnimMontage* Montage);

	ASwordslikeCharacter* OwnerCharacter;
};
