#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "PlayerSelectionMenuWidget.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(CharacterNameEvent, const FString&);

class UPlayerStartCharacterDataAsset;
class AMainPlayerState;
class UProgressBar;
class UEditableText;
class UImage;
class UTextBlock;

/**
 * 
 */
UCLASS()
class SWORDSLIKE_API UPlayerSelectionMenuWidget : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ReadyText;
	
	UPROPERTY(meta = (BindWidget))
	UImage* CharacterImage;
	
	UPROPERTY(meta = (BindWidget))
	UEditableText* PlayerNameTextBox;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* PostureBar;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CharacterNameText;
	
	bool bIsEditingName = false;

	const int32 NameMaxCharacters = 12;

protected:
	virtual void NativeConstruct() override;
	
public:
	void InitWithPlayerState(AMainPlayerState* PlayerState);
	void UpdateWithCharacterData(const TSoftObjectPtr<UPlayerStartCharacterDataAsset>& Data);
	void OnLocalPlayerController();
	void OnRemotePlayerController();
	void DisplayReadyText(const bool bDisplay);
	FORCEINLINE void OnReady() {DisplayReadyText(true);}
	FORCEINLINE void OnNotReady() {DisplayReadyText(false);}

	CharacterNameEvent OnPlayerNameChanged;
	
	UFUNCTION()
	void HandleNameChanged(const FText& Text);
	UFUNCTION()
	void HandleNameCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	void SetPlayerName(const FString& NewName);
	
	FORCEINLINE bool IsEditingName() const { return bIsEditingName; }
	FORCEINLINE FString GetNameText(){ return PlayerNameTextBox->GetText().ToString(); }

	static float MaxHealth;
	static float MaxStamina;
	static float MaxPosture;
};