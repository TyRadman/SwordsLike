#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Swordslike/UI/Lobby/MainLobbyMenu.h"
#include "MainPlayerController.generated.h"

class UInputAction;
class UInputMappingContext;
class UMainLobbyMenu;
class UPlayerStartCharacterDataAsset;

/**
 * 
 */
UCLASS()
class SWORDSLIKE_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void ClientReset_Implementation() override;
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<AHUD> HUDToUse;

// 	virtual void SetupInputComponent() override;
//
// 	UPROPERTY(EditDefaultsOnly, Category=Input, meta=(AllowPrivateAccess))
// 	UInputMappingContext* MappingContext;
// 	UPROPERTY(EditDefaultsOnly, Category=Input, meta=(AllowPrivateAccess))
// 	UInputAction* LeftInputAction;
// 	UPROPERTY(EditDefaultsOnly, Category=Input, meta=(AllowPrivateAccess))
// 	UInputAction* RightInputAction;
// 	
// 	UPROPERTY(EditDefaultsOnly, Category = UI, meta=(AllowPrivateAccess))
// 	TSubclassOf<UMainLobbyMenu> MainLobbyUIClass;
// 	
// 	void OnSelectLeft();
// 	void OnSelectRight();
//
// 	UMainLobbyMenu* LobbyUI;
// 	
//
// public:
// 	UFUNCTION(Server, Reliable)
// 	void Server_SelectCharacter(const TSoftObjectPtr<UPlayerStartCharacterDataAsset>& SelectedAsset);

};
