#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Swordslike/UI/Lobby/MainLobbyMenu.h"
#include "LobbyPlayerPawn.generated.h"

class AMainPlayerState;
class UInputAction;
class UInputMappingContext;

UCLASS()
class SWORDSLIKE_API ALobbyPlayerPawn : public APawn
{
	GENERATED_BODY()

public:
	ALobbyPlayerPawn();

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void OnRep_PlayerState() override;

	void Test();

	UPROPERTY(EditDefaultsOnly, Category=Input, meta=(AllowPrivateAccess))
	UInputMappingContext* MappingContext;
	UPROPERTY(EditDefaultsOnly, Category=Input, meta=(AllowPrivateAccess))
	UInputAction* LeftInputAction;
	UPROPERTY(EditDefaultsOnly, Category=Input, meta=(AllowPrivateAccess))
	UInputAction* RightInputAction;
	UPROPERTY(EditDefaultsOnly, Category=Input, meta=(AllowPrivateAccess))
	UInputAction* ConfirmInputAction;
	UPROPERTY(EditDefaultsOnly, Category=Input, meta=(AllowPrivateAccess))
	UInputAction* ReturnInputAction;
	
	UPROPERTY(EditDefaultsOnly, Category = UI, meta=(AllowPrivateAccess))
	TSubclassOf<UMainLobbyMenu> MainLobbyUIClass;
	
	void OnSelectLeft();
	void OnSelectRight();
	void ConfirmSelection();
	void ReturnFromSelection();

	UPROPERTY(Replicated)
	UMainLobbyMenu* LobbyUI;

	bool bIsSelectionReady = false;

	void SaveCurrentCharacterData();
	void PerformSaveCurrentCharacterData(TSoftObjectPtr<UPlayerStartCharacterDataAsset> Data);
	
	UFUNCTION(Server, Reliable)
	void Server_SelectCharacter(const TSoftObjectPtr<UPlayerStartCharacterDataAsset>& Data);

};
