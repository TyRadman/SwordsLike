#include "PlayerSelectionMenuWidget.h"

#include "Components/EditableText.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Player/MainPlayerState.h"

class UPlayerStartCharacterDataAsset;

float UPlayerSelectionMenuWidget::MaxHealth = 100.f;
float UPlayerSelectionMenuWidget::MaxStamina = 100.f;
float UPlayerSelectionMenuWidget::MaxPosture = 100.f;

void UPlayerSelectionMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UPlayerSelectionMenuWidget::InitWithPlayerState(AMainPlayerState* PlayerState)
{
	if (!PlayerState)
	{
		return;
	}
	
	PlayerNameTextBox->SetText(FText::FromString(PlayerState->GetPlayerName()));

	if (const UPlayerStartCharacterDataAsset* DataAsset = PlayerState->GetCurrentDataAsset())
	{
		UpdateWithCharacterData(DataAsset);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No data 1"));
	}
}

void UPlayerSelectionMenuWidget::UpdateWithCharacterData(const TSoftObjectPtr<UPlayerStartCharacterDataAsset>& Data)
{
	if (Data)
	{
		CharacterNameText->SetText(FText::FromString(Data->CharacterName));
		CharacterImage->SetBrushFromTexture(Data->CharacterTexture);
		HealthBar->SetPercent(Data->StartingHealthPoints / MaxHealth);
		StaminaBar->SetPercent(Data->StartingStamina / MaxStamina);
		PostureBar->SetPercent(Data->StartingPosture / MaxPosture);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No data 2"));
	}
}

void UPlayerSelectionMenuWidget::OnLocalPlayerController()
{
	PlayerNameTextBox->SetIsReadOnly(false);
	PlayerNameTextBox->OnTextCommitted.AddDynamic(this, &UPlayerSelectionMenuWidget::HandleNameCommitted);
	PlayerNameTextBox->OnTextChanged.AddDynamic(this, &UPlayerSelectionMenuWidget::HandleNameChanged);

}

void UPlayerSelectionMenuWidget::OnRemotePlayerController()
{
	PlayerNameTextBox->SetIsReadOnly(true);
	PlayerNameTextBox->SetIsEnabled(false);
	// remove the tooltip text from the other players' textboxes
	PlayerNameTextBox->SetHintText(FText::FromString("Player Name"));
}

void UPlayerSelectionMenuWidget::DisplayReadyText(const bool bDisplay)
{
	if (bIsEditingName)
	{
		bIsEditingName = false;
		PlayerNameTextBox->SetKeyboardFocus();
		return;
	}

	ReadyText->SetVisibility(bDisplay? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UPlayerSelectionMenuWidget::HandleNameChanged(const FText& Text)
{
	bIsEditingName = true;

	// ensure the name is within the chars limit
	if(const FString Content = Text.ToString(); Content.Len() > NameMaxCharacters)
	{
		const FString TrimmedName = Content.Left(NameMaxCharacters);
		PlayerNameTextBox->SetText(FText::FromString(TrimmedName));
	}

	// fire the event to update the text field on other clients
	if(OnPlayerNameChanged.IsBound())
	{
		OnPlayerNameChanged.Broadcast(PlayerNameTextBox->GetText().ToString());
	}
}

void UPlayerSelectionMenuWidget::HandleNameCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	bIsEditingName = false;
}

void UPlayerSelectionMenuWidget::SetPlayerName(const FString& NewName)
{
	PlayerNameTextBox->SetText(FText::FromString(NewName));
}

