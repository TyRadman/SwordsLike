#include "LobbyPlayerPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MainPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Swordslike/SwordslikeGameInstance.h"
#include "Swordslike/UI/Lobby/LobbyHUD.h"

class USwordslikeGameInstance;
class UEnhancedInputLocalPlayerSubsystem;

ALobbyPlayerPawn::ALobbyPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALobbyPlayerPawn::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerPawn, LobbyUI);
	DOREPLIFETIME(ALobbyPlayerPawn, bIsSelectionReady);
}

void ALobbyPlayerPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void ALobbyPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle;
	// GetWorldTimerManager().SetTimerForNextTick(this, &ALobbyPlayerPawn::Test);
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ALobbyPlayerPawn::Test, 5.f, false);
}

void ALobbyPlayerPawn::Test()
{
	if(const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(IsLocallyControlled())
		{
			const FString Message = FString::Printf(TEXT("Setting up %s [%s]"), *PlayerController->GetActorNameOrLabel(), *UEnum::GetValueAsString(GetLocalRole()));
			GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, Message);
			
			if (const ALobbyHUD* HUD = Cast<ALobbyHUD>(PlayerController->GetHUD()))
			{
				LobbyUI = HUD->GetLobbyUI();
				LobbyUI->SetupPlayerWidgets();
				LobbyUI->BindWidgets();
			}
			else
			{
			const FString Message2 = FString::Printf(TEXT("No HUD on  %s [%s]"), *PlayerController->GetActorNameOrLabel(), *UEnum::GetValueAsString(GetLocalRole()));
			GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, Message2);
			}
		}
		else
		{
			const FString Message = FString::Printf(TEXT("No a local controller %s [%s]"), *PlayerController->GetActorNameOrLabel(), *UEnum::GetValueAsString(GetLocalRole()));
			GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, Message);
		}
	}
}

void ALobbyPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (MappingContext)
			{
				InputSubsystem->AddMappingContext(MappingContext, 0);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, "No MappingContext");
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, "No UEnhancedInputLocalPlayerSubsystem");
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, "No APlayerController");
	}
	
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		Input->BindAction(LeftInputAction, ETriggerEvent::Started, this, &ALobbyPlayerPawn::OnSelectLeft);
		Input->BindAction(RightInputAction, ETriggerEvent::Started, this, &ALobbyPlayerPawn::OnSelectRight);
		Input->BindAction(ConfirmInputAction, ETriggerEvent::Started, this, &ALobbyPlayerPawn::ConfirmSelection);
		Input->BindAction(ReturnInputAction, ETriggerEvent::Started, this, &ALobbyPlayerPawn::ReturnFromSelection);
	}
}

void ALobbyPlayerPawn::OnSelectLeft()
{
	if(bIsSelectionReady)
	{
		return;
	}
	
	if(LobbyUI)
	{
		LobbyUI->SelectPreviousCharacter();
		SaveCurrentCharacterData();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, "No hud LobbyUI");
	}
}

void ALobbyPlayerPawn::OnSelectRight()
{
	if(bIsSelectionReady)
	{
		return;
	}
	
	if(LobbyUI)
	{
		LobbyUI->SelectNextCharacter();
		SaveCurrentCharacterData();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, "No hud LobbyUI");
	}
}

void ALobbyPlayerPawn::SaveCurrentCharacterData()
{
	const TSoftObjectPtr<UPlayerStartCharacterDataAsset> Data = LobbyUI->GetSelectedCharacter();
	
	if(!HasAuthority())
	{
		Server_SelectCharacter(Data);
	}
	else
	{
		PerformSaveCurrentCharacterData(Data);
	}
}

void ALobbyPlayerPawn::PerformSaveCurrentCharacterData(TSoftObjectPtr<UPlayerStartCharacterDataAsset> Data)
{
	if(Data)
	{
		if(AMainPlayerState* State = GetPlayerState<AMainPlayerState>())
		{
			State->SetCharacterDataAsset(Data.LoadSynchronous());
			State->OnRep_CharacterDataAsset();
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Purple, "No CharacterDataAsset");
	}
}

void ALobbyPlayerPawn::Server_SelectCharacter_Implementation(const TSoftObjectPtr<UPlayerStartCharacterDataAsset>& Data)
{
	PerformSaveCurrentCharacterData(Data);

	if(AMainPlayerState* State = GetPlayerState<AMainPlayerState>())
	{
		State->OnRep_CharacterDataAsset();
	}
}

#pragma region Confirmation And Returning
void ALobbyPlayerPawn::ConfirmSelection()
{
	if(!LobbyUI)
	{
		GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, "No hud LobbyUI");
		return;
	}
	
	if(const auto State = GetMainPlayerState())
	{
		if(USwordslikeGameInstance* GI = Cast<USwordslikeGameInstance>(GetGameInstance()))
		{
			if(State->CharacterDataAsset)
			{
				GI->SetLocalCharacterData(State->CharacterDataAsset);
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Blue, FString::Printf(TEXT("No DATA!!!")));
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 100.f, FColor::Blue, FString::Printf(TEXT("No GI!!!")));
		}
	}

	LobbyUI->ConfirmSelection();

	if(!HasAuthority())
	{
		Server_ConfirmSelection();
	}
	else
	{
		PerformConfirmSelection();
	}
}

void ALobbyPlayerPawn::Server_ConfirmSelection_Implementation()
{
	PerformConfirmSelection();
}

void ALobbyPlayerPawn::PerformConfirmSelection()
{
	if(bIsSelectionReady)
	{
		return;
	}
	
	bIsSelectionReady = true;
	OnRep_bIsSelectionReady();
}

void ALobbyPlayerPawn::ReturnFromSelection()
{
	if(!LobbyUI)
	{
		GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, "No hud LobbyUI");
		return;
	}
	
	LobbyUI->ReturnFromSelection();

	if(!HasAuthority())
	{
		Server_ReturnFromSelection();
	}
	else
	{
		PerformReturnFromSelection();
	}
}

void ALobbyPlayerPawn::Server_ReturnFromSelection_Implementation()
{
	PerformReturnFromSelection();
}

void ALobbyPlayerPawn::PerformReturnFromSelection()
{
	if(!bIsSelectionReady)
	{
		return;
	}
	
	bIsSelectionReady = false;
	OnRep_bIsSelectionReady();
}

void ALobbyPlayerPawn::OnRep_bIsSelectionReady()
{
	if(auto State = GetMainPlayerState())
	{
		if(bIsSelectionReady)
		{
			State->OnPlayerReady.Broadcast();
		}
		else
		{
			State->OnPlayerNotReady.Broadcast();
		}
	}
}
#pragma endregion

AMainPlayerState* ALobbyPlayerPawn::GetMainPlayerState() const
{
	if(AMainPlayerState* State = GetPlayerState<AMainPlayerState>())
	{
		return State;
	}
		
	GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, "NO STATE FOUND!!");
	return nullptr;
}