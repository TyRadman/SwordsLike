#include "MainPlayerController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MainPlayerState.h"
#include "PlayerStartCharacterDataAsset.h"
#include "Swordslike/UI/Lobby/MainLobbyMenu.h"

// class UEnhancedInputLocalPlayerSubsystem;
//
// void AMainPlayerController::BeginPlay()
// {
// 	Super::BeginPlay();
//
// 	const FString PlayerName = GetPlayerState<AMainPlayerState>() ? GetPlayerState<AMainPlayerState>()->GetPlayerName() : TEXT("Unknown");
// 	const FString Message = FString::Printf(TEXT("Controller BeginPlay: %s | Local: %s | Server: %s"),
// 	                                        *PlayerName,
// 	                                        IsLocalController() ? TEXT("YES") : TEXT("NO"),
// 	                                        HasAuthority() ? TEXT("YES") : TEXT("NO"));
// 	
// 	GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Green, Message);
//
// 	if (IsLocalController())
// 	{
// 		LobbyUI = CreateWidget<UMainLobbyMenu>(this, MainLobbyUIClass);
// 		if(LobbyUI)
// 		{
// 			LobbyUI->AddToViewport();
// 			LobbyUI->SetVisibility(ESlateVisibility::Visible);
// 		GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Green, FString::Printf(TEXT("Lobby created for controller with name: %s"), *PlayerName));
// 		}
// 		else
// 		{
// 			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "LobbyUI could not be created");
// 		}
// 	}
// }
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
