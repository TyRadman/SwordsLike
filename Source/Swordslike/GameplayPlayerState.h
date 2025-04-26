#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayPlayerState.generated.h"

UCLASS()
class SWORDSLIKE_API AGameplayPlayerState : public APlayerState
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
};
