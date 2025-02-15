// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LockWidgetController.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API ULockWidgetController : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ULockWidgetController();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, Category="Lock-On")
	TSubclassOf<AActor> LockOnIndicatorClass;

	UPROPERTY()
	AActor* LockOnIndicatorInstance;

	void LockOnTarget(AActor* Target);
	void UnlockFromTarget();
		
};
