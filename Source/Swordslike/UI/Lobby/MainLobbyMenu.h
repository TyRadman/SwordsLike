#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainLobbyMenu.generated.h"

class AMainPlayerState;
class UPlayerStartCharacterDataAsset;
class UPlayerSelectionMenuWidget;
class UUniformGridPanel;
/**
 * Holds the character selection boxes of all the players in the lobby.
 */
UCLASS()
class SWORDSLIKE_API UMainLobbyMenu : public UUserWidget
{
	GENERATED_BODY()


protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UUniformGridPanel* PlayersGrid;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = true))
	TSubclassOf<UPlayerSelectionMenuWidget> PlayersWidgetClass;

	TMap<AMainPlayerState*, UPlayerSelectionMenuWidget*> PlayerWidgetsMap;

	
private:
	const int8 MaxRowsCount = 2;
	const int8 MaxColsCount = 4;

	TArray<TSoftObjectPtr<UPlayerStartCharacterDataAsset>> AvailableCharacters;
	int32 SelectedCharacterIndex = 0;

	void SetSelectedCharacter(const TSoftObjectPtr<UPlayerStartCharacterDataAsset>& Character);
	UPlayerSelectionMenuWidget* LocalPlayerCharacterWidget;

public:
	void Refresh();
	/**
	 * Updates the player name on the Autonomous client side whenever it's changed on other game instances.
	 * @param NewName New Player Name
	 */
	void UpdateNameText(const FString& NewName);
	void SetupPlayerWidgets();
	
	void BindWidgets();
	void SelectNextCharacter();
	void SelectPreviousCharacter();
	void ConfirmSelection();
	void ReturnFromSelection();
	FORCEINLINE TSoftObjectPtr<UPlayerStartCharacterDataAsset> GetSelectedCharacter() const {return AvailableCharacters[SelectedCharacterIndex]; }

	bool CanBeConfirmed() const;
};
