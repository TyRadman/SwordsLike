// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "LockWidgetController.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API ULockWidgetController : public UMyActorComponent, public IIEntityComponent
{
	GENERATED_BODY()

public:	
	ULockWidgetController();
	virtual void InitEntityComponent(ACharacter* Character) override;

	UPROPERTY(EditDefaultsOnly, Category="Lock-On")
	TSubclassOf<AActor> LockOnIndicatorClass;

	UPROPERTY()
	AActor* LockOnIndicatorInstance;

	void ShowIndicatorOnTarget(USceneComponent* Target);
	void HideIndicator();

private:
	bool IsLocallyControlledActor() const;
};
