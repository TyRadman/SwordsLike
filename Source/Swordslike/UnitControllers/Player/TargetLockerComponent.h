// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "TargetLockerComponent.generated.h"

class UInputMappingContext;
class UInputAction;
class ULockWidgetController;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SWORDSLIKE_API UTargetLockerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTargetLockerComponent();

protected:
	// Called when the game starts.
	virtual void BeginPlay() override;

public:	
	// Called every frame.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LockAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "References", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<ULockWidgetController> LockIndicatorWidget;

	void SetLockIndicatorWidget(TObjectPtr<ULockWidgetController> LockIndicatorWidget);
	

private:
	void LockToTarget();

	TObjectPtr<AActor> LockedTarget;
	TObjectPtr<AController> CharacterController;
	TObjectPtr<USpringArmComponent> SpringArm;

	bool IsLockedOnTarget = false;
};
