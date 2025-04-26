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

UCLASS()
class SWORDSLIKE_API  AMainPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void CopyProperties(APlayerState* PlayerState) override;
	
public:
	EPlayerPhase PlayerPhase = EPlayerPhase::Lobby;

	virtual void OnRep_PlayerName() override;

	void SetPlayerNewName(const FString& NewName);
	UFUNCTION(Server, Reliable)
	void Server_SetPlayerNewName(const FString& NewName);
	void PerformSetPlayerNewName(const FString& NewName);

	CharacterEvent OnCharacterChangedEvent;
	CharacterNameEvent OnPlayerNameChanged;
	CharacterEvent OnPlayerReady;
	CharacterEvent OnPlayerNotReady;


	FORCEINLINE void SetCharacterDataAsset(UPlayerStartCharacterDataAsset* NewCharacterDataAsset) { CharacterDataAsset = NewCharacterDataAsset; }
	FORCEINLINE UPlayerStartCharacterDataAsset* GetCurrentDataAsset() const { return CharacterDataAsset; }
	
	UPROPERTY(ReplicatedUsing = OnRep_CharacterDataAsset)
	UPlayerStartCharacterDataAsset* CharacterDataAsset;
	UFUNCTION()
	void OnRep_CharacterDataAsset();

	UPROPERTY(ReplicatedUsing=OnRep_StateIndex)
	int32 StateIndex = -1;

protected:
	UFUNCTION()
	void OnRep_StateIndex();
};
