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
	void OnSelectLeft();
	void OnSelectRight();

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

	

	void ConfirmSelection();
	UFUNCTION(Server, Reliable)
	void Server_ConfirmSelection();
	void PerformConfirmSelection();
	void ReturnFromSelection();
	UFUNCTION(Server, Reliable)
	void Server_ReturnFromSelection();
	void PerformReturnFromSelection();

	UPROPERTY(Replicated)
	UMainLobbyMenu* LobbyUI;

	UPROPERTY(ReplicatedUsing = OnRep_bIsSelectionReady)
	bool bIsSelectionReady = false;
	UFUNCTION()
	void OnRep_bIsSelectionReady();

	void SaveCurrentCharacterData();
	void PerformSaveCurrentCharacterData(TSoftObjectPtr<UPlayerStartCharacterDataAsset> Data);
	
	UFUNCTION(Server, Reliable)
	void Server_SelectCharacter(const TSoftObjectPtr<UPlayerStartCharacterDataAsset>& Data);

	FORCEINLINE AMainPlayerState* GetMainPlayerState() const;
};
