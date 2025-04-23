#include "MainPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MainPlayerState.h"
#include "PlayerStartCharacterDataAsset.h"
#include "Swordslike/UI/Lobby/LobbyHUD.h"
#include "Swordslike/UI/Lobby/MainLobbyMenu.h"

void AMainPlayerController::ClientReset_Implementation()
{
	Super::ClientReset_Implementation();
	UE_LOG(LogTemp, Warning, TEXT("ClientRestart: %s, HasHUD: %s, HUDToUse: %s"),
		*GetName(),
		GetHUD() ? TEXT("YES") : TEXT("NO"),
		*GetNameSafe(HUDToUse));

	if (IsLocalController() && GetHUD() == nullptr && HUDToUse)
	{
		ClientSetHUD(HUDToUse);
	}

	// 	if (IsLocalController() && GetHUD() == nullptr && HUDToUse)
	// 	{
	// 		ClientSetHUD(HUDToUse); // ← this line makes use of the variable you assigned
	// 	}
	//
	//
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT(" [%s]"), *HUDToUse->GetClass()->GetName()));

}

// class UEnhancedInputLocalPlayerSubsystem;
//
void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay: %s, HasHUD: %s, HUDToUse: %s"),
		*GetName(),
		GetHUD() ? TEXT("YES") : TEXT("NO"),
		*GetNameSafe(HUDToUse));
	
	// const FString PlayerName = GetPlayerState<AMainPlayerState>() ? GetPlayerState<AMainPlayerState>()->GetPlayerName() : TEXT("Unknown");
	// const FString Message = FString::Printf(TEXT("Controller BeginPlay: %s | Local: %s | Server: %s"),
	//                                         *PlayerName,
	//                                         IsLocalController() ? TEXT("YES") : TEXT("NO"),
	//                                         HasAuthority() ? TEXT("YES") : TEXT("NO"));
	//
	// GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Green, Message);
	//
	// if (IsLocalController())
	// {
	// 	LobbyUI = CreateWidget<UMainLobbyMenu>(this, MainLobbyUIClass);
	// 	if(LobbyUI)
	// 	{
	// 		LobbyUI->AddToViewport();
	// 		LobbyUI->SetVisibility(ESlateVisibility::Visible);
	// 	GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Green, FString::Printf(TEXT("Lobby created for controller with name: %s"), *PlayerName));
	// 	}
	// 	else
	// 	{
	// 		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "LobbyUI could not be created");
	// 	}
	// }
}
//
// void AMainPlayerController::SetupInputComponent()
// {
// 	Super::SetupInputComponent();
//
// 	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
// 	{
// 		Input->BindAction(LeftInputAction, ETriggerEvent::Triggered, this, &AMainPlayerController::OnSelectLeft);
// 		Input->BindAction(RightInputAction, ETriggerEvent::Triggered, this, &AMainPlayerController::OnSelectRight);
// 	}
// }
//
// void AMainPlayerController::OnSelectLeft()
// {
// 	if(LobbyUI)
// 	{
// 		LobbyUI->SelectPreviousCharacter();
// 	}
// }
//
// void AMainPlayerController::OnSelectRight()
// {
// 	if(LobbyUI)
// 	{
// 		LobbyUI->SelectNextCharacter();
// 	}
// }
//
// void AMainPlayerController::Server_SelectCharacter_Implementation(const TSoftObjectPtr<UPlayerStartCharacterDataAsset>& SelectedAsset)
// {
// 	// your logic
// }
