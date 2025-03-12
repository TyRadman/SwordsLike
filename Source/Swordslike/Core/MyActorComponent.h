// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyActorComponent.generated.h"


UCLASS(Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API UMyActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMyActorComponent();

protected:
	virtual void BeginPlay() override;

	void PrintOnScreen_Local(const FString& Message) const;
	void PrintOnScreen_Local(const FString& Message, FColor Color) const;
	void PrintOnScreen_Local(const FString& Message, FColor Color, float Duration) const;
	void PrintOnScreen_Local(const FString& Message, float Duration) const;
	void PrintOnScreen(const FString& Message) const;
	void PrintOnScreen(const FString& Message, FColor Color) const;
	void PrintOnScreen(const FString& Message, FColor Color, float Duration) const;

	/**
	 * 
	 * @return True, if the owner is the server.
	 */
	bool HasAuthority() const;
	bool IsAutonomousProxy() const;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
