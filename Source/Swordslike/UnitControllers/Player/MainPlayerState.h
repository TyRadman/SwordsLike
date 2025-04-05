#pragma once

#include "CoreMinimal.h"
#include "PlayerStartCharacterDataAsset.h"
#include "GameFramework/PlayerState.h"
#include "MainPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE(CharacterEvent);
DECLARE_MULTICAST_DELEGATE_OneParam(CharacterNameEvent, const FString&);

UENUM(BlueprintType)
enum class EPlayerPhase : uint8
{
	Lobby = 0,
	GamePlay = 1
};

/**
 * 
 */
UCLASS()
class SWORDSLIKE_API AMainPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	
	UPROPERTY(ReplicatedUsing = OnRep_PlayerPhase)
	EPlayerPhase PlayerPhase = EPlayerPhase::Lobby;
	UFUNCTION()
	void OnRep_PlayerPhase();

	virtual void OnRep_PlayerName() override;

	void SetPlayerNewName(const FString& NewName);
	UFUNCTION(Server, Reliable)
	void Server_SetPlayerNewName(const FString& NewName);
	void PerformSetPlayerNewName(const FString& NewName);
	// FORCEINLINE void SetPlayerName(const FString& NewPlayerName) { SetPlayerName(NewPlayerName);}

	CharacterEvent OnCharacterChangedEvent;
	CharacterNameEvent OnPlayerNameChanged;


	FORCEINLINE void SetCharacterDataAsset(const TSoftObjectPtr<UPlayerStartCharacterDataAsset>& NewCharacterDataAsset) { CharacterDataAsset = NewCharacterDataAsset; }
	FORCEINLINE UPlayerStartCharacterDataAsset* GetCurrentDataAsset() const { return CharacterDataAsset.Get(); }
	
	UFUNCTION()
	void OnRep_CharacterDataAsset();
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	UPROPERTY(ReplicatedUsing = OnRep_CharacterDataAsset)
	TSoftObjectPtr<UPlayerStartCharacterDataAsset> CharacterDataAsset;
};
