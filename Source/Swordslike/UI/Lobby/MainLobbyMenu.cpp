#include "MainLobbyMenu.h"

#include "PlayerSelectionMenuWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "GameFramework/GameStateBase.h"
#include "Player/LobbyPlayerPawn.h"
#include "Player/MainPlayerState.h"
#include "Swordslike/SwordslikeGameInstance.h"
#include "Swordslike/GameStates/LobbyGameState.h"
#include "Swordslike/Utilities/UtilHelper.h"

class AMainPlayerController;

void UMainLobbyMenu::NativeConstruct()
{
	Super::NativeConstruct();

	if(const USwordslikeGameInstance* Instance = GetGameInstance<USwordslikeGameInstance>())
	{
		for(TSoftObjectPtr<UPlayerStartCharacterDataAsset> DA : Instance->PlayerCharactersData)
		{
			AvailableCharacters.Add(DA.LoadSynchronous());
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "ERROR: No custom game instance!");
	}

	if(GetWorld())
	{
		UUtilHelper::ShowCursor(GetWorld(), this, false);
	}
	
	PlayerWidgetsMap.Empty();

	TArray<UWidget*> Children = PlayersGrid->GetAllChildren();
	for (UWidget* Child : Children)
	{
		if (UPlayerSelectionMenuWidget* PlayerWidget = Cast<UPlayerSelectionMenuWidget>(Child))
		{
			PlayerWidget->SetVisibility(ESlateVisibility::Hidden);
			PlayerWidgetsList.Add(PlayerWidget);
		}
	}
}

void UMainLobbyMenu::SetupPlayerWidgets()
{
	if (!GetWorld())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "UMainLobbyMenu ERROR: no world!");
		return;
	}
	
	ALobbyGameState* GameState = Cast<ALobbyGameState>(GetWorld()->GetGameState());
	if (!GameState || !PlayersWidgetClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "UMainLobbyMenu ERROR: no game state or playersWidgetClass!");
		return;
	}

	int Index = 0;
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (AMainPlayerState* MainPS = Cast<AMainPlayerState>(PlayerState))
		{
			UPlayerSelectionMenuWidget* CharacterWidget = PlayerWidgetsList[MainPS->StateIndex];
			CharacterWidget->SetVisibility(ESlateVisibility::Visible);
			PlayerWidgetsMap.Add(MainPS, CharacterWidget);

			// populate and cache the character widget if they own it
			if (MainPS == GetOwningPlayer()->PlayerState)
			{
				if(ALobbyPlayerPawn* PlayerPawn = Cast<ALobbyPlayerPawn>(GetOwningPlayer()->GetPawn()))
				{
					CharacterWidget->OwnerPawn = PlayerPawn;
				}
				
				CharacterWidget->OnLocalPlayerController();
				LocalPlayerCharacterWidget = CharacterWidget;
			}
			// otherwise, disable the widget as it belongs to another player
			else
			{
				CharacterWidget->OnRemotePlayerController();
			}

			Index++;
		}
	}

	Refresh();
}

void UMainLobbyMenu::Refresh()
{
	for (const TPair<AMainPlayerState*, UPlayerSelectionMenuWidget*>& Pair : PlayerWidgetsMap)
	{
		if (Pair.Key && Pair.Value && Pair.Key->GetCurrentDataAsset())
		{
			Pair.Value->InitWithPlayerState(Pair.Key);
		}
	}
}

void UMainLobbyMenu::UpdateNameText(const FString& NewName)
{
	for (const TPair<AMainPlayerState*, UPlayerSelectionMenuWidget*>& Pair : PlayerWidgetsMap)
	{
		if (Pair.Key && Pair.Value && Pair.Key->GetCurrentDataAsset())
		{
			Pair.Value->SetPlayerName(Pair.Key->GetPlayerName());
		}
	}
}

void UMainLobbyMenu::BindWidgets()
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "ERROR: No game state!");
		return;
	}

	auto PlayerStates = GameState->PlayerArray;
	for (APlayerState* PlayerState : PlayerStates)
	{
		AMainPlayerState* MainPlayerState = Cast<AMainPlayerState>(PlayerState);
		if (!MainPlayerState)
		{
			GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Yellow, "UMainLobbyMenu ERROR: No Character AMainPlayerState");
			continue;
		}

		MainPlayerState->OnCharacterChangedEvent.AddUObject(this, &UMainLobbyMenu::Refresh);

		MainPlayerState->OnPlayerNameChanged.AddUObject(this, &UMainLobbyMenu::UpdateNameText);

		UPlayerSelectionMenuWidget* CharacterWidget = PlayerWidgetsMap[MainPlayerState];
		
		MainPlayerState->OnPlayerReady.AddUObject(CharacterWidget, &UPlayerSelectionMenuWidget::OnReady);
		MainPlayerState->OnPlayerNotReady.AddUObject(CharacterWidget, &UPlayerSelectionMenuWidget::OnNotReady);

		CharacterWidget->OnPlayerNameChanged.AddUObject(MainPlayerState, &AMainPlayerState::SetPlayerNewName);
	}
}

void UMainLobbyMenu::SelectNextCharacter()
{
	if (AvailableCharacters.Num() == 0 || !LocalPlayerCharacterWidget)
	{
		return;
	}
	
	SelectedCharacterIndex = (SelectedCharacterIndex + 1) % AvailableCharacters.Num();
	SetSelectedCharacter(AvailableCharacters[SelectedCharacterIndex].Get());
}

void UMainLobbyMenu::SelectPreviousCharacter()
{
	if (AvailableCharacters.Num() == 0 || !LocalPlayerCharacterWidget)
	{
		return;
	}
	
	SelectedCharacterIndex = (SelectedCharacterIndex - 1 + AvailableCharacters.Num()) % AvailableCharacters.Num();
	SetSelectedCharacter(AvailableCharacters[SelectedCharacterIndex]);
}

void UMainLobbyMenu::ConfirmSelection()
{
	if (!LocalPlayerCharacterWidget)
	{
		return;
	}
	
	LocalPlayerCharacterWidget->DisplayReadyText(true);
}

void UMainLobbyMenu::ReturnFromSelection()
{
	if (!LocalPlayerCharacterWidget)
	{
		return;
	}

	if(LocalPlayerCharacterWidget)
	{
		LocalPlayerCharacterWidget->DisplayReadyText(false);
	}
}

bool UMainLobbyMenu::CanBeConfirmed() const
{
	if(!LocalPlayerCharacterWidget)
	{
		return false;
	}
	
	return !LocalPlayerCharacterWidget->IsEditingName();
}

void UMainLobbyMenu::SetSelectedCharacter(const TSoftObjectPtr<UPlayerStartCharacterDataAsset>& Character)
{
	if (!Character || !LocalPlayerCharacterWidget)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("UMainLobbyMenu::SetSelectedCharacter ERROR: no Character or LocalPlayerWidget")));
		return;
	}
	
	LocalPlayerCharacterWidget->UpdateWithCharacterData(Character);
}
