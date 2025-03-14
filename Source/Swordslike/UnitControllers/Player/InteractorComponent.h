#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractorComponent.generated.h"

// TODO: remove this
UCLASS()
class SWORDSLIKE_API AInteractorComponent : public AActor
{
	GENERATED_BODY()
	
public:	
	AInteractorComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

};
