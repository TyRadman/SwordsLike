#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GameplayPlayerState.generated.h"

class ASwordslikeCharacter;

UCLASS()
class SWORDSLIKE_API AGameplayPlayerState : public APlayerState
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	virtual void OnRep_PlayerName() override;

	UFUNCTION(Server, Reliable)
	void Server_SetPlayerName(const FString& NameToSet);

	ASwordslikeCharacter* PlayerCharacter;
};
