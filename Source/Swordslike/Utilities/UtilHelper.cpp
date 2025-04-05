#include "UtilHelper.h"

#include "Components/Widget.h"
#include "Player/SwordslikeCharacter.h"

void UUtilHelper::ShowCursor(const UWorld* World, UWidget* Widget, const bool bOnlyCursorControls)
{
	if (const UGameInstance* GameInstance = World->GetGameInstance())
	{
		if (const ULocalPlayer* LocalPlayer = GameInstance->GetFirstGamePlayer())
		{
			if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(World))
			{
				if(bOnlyCursorControls)
				{
					FInputModeUIOnly InputMode;
					InputMode.SetWidgetToFocus(Widget->TakeWidget());
					InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
					PlayerController->SetInputMode(InputMode);
				}
				else
				{
					FInputModeGameAndUI InputMode;
					InputMode.SetWidgetToFocus(Widget->TakeWidget());
					InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
					PlayerController->SetInputMode(InputMode);
				}

				PlayerController->SetShowMouseCursor(true);
			}
		}
	}
}

void UUtilHelper::HideCursor(const UWorld* World)
{
	if (const UGameInstance* GameInstance = World->GetGameInstance())
	{
		if (const ULocalPlayer* LocalPlayer = GameInstance->GetFirstGamePlayer())
		{
			if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(World))
			{
				const FInputModeGameOnly InputMode;
				PlayerController->SetInputMode(InputMode);
				PlayerController->SetShowMouseCursor(false);
			}
		}
	}
}
