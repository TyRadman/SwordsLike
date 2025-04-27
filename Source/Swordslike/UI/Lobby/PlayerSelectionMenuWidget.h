#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "PlayerSelectionMenuWidget.generated.h"

class ALobbyPlayerPawn;
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
	UProgressBar* ToughnessBar;
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

	/**
	 * Set to BlueprintNativeEvent so that we add obvious functionality in BPs.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnLocalPlayerController();
	virtual void OnLocalPlayerController_Implementation();
	/**
	 * Called when the widget created isn't owned by the player in control.
	 */
	UFUNCTION(BlueprintNativeEvent)
	void OnRemotePlayerController();
	virtual void OnRemotePlayerController_Implementation();
	
	void DisplayReadyText(const bool bDisplay);
	FORCEINLINE void OnReady() {DisplayReadyText(true);}
	FORCEINLINE void OnNotReady() {DisplayReadyText(false);}

	CharacterNameEvent OnPlayerNameChanged_Widget;
	
	UFUNCTION()
	void HandleNameChanged(const FText& Text);
	UFUNCTION()
	void HandleNameCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION(BlueprintCallable)
	void SetNextCharacter();
	UFUNCTION(BlueprintCallable)
	void SetPreviousCharacter();

	void SetPlayerName(const FString& NewName);
	
	FORCEINLINE bool IsEditingName() const { return bIsEditingName; }
	FORCEINLINE FString GetNameText(){ return PlayerNameTextBox->GetText().ToString(); }
	
	ALobbyPlayerPawn* OwnerPawn;

	static float MaxToughness;
	static float MaxHealth;
	static float MaxStamina;
	static float MaxPosture;
};