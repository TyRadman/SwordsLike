#include "LobbyPlayerPawn.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MainPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Swordslike/UI/Lobby/LobbyHUD.h"

class UEnhancedInputLocalPlayerSubsystem;

ALobbyPlayerPawn::ALobbyPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ALobbyPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	FTimerHandle TimerHandle;
	// GetWorldTimerManager().SetTimerForNextTick(this, &ALobbyPlayerPawn::Test);
	GetWorldTimerManager().SetTimer(TimerHandle, this, &ALobbyPlayerPawn::Test, 5.f, false);
}

void ALobbyPlayerPawn::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void ALobbyPlayerPawn::Test()
{
	if(const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(PlayerController->IsLocalController())
		{
			if (const ALobbyHUD* HUD = Cast<ALobbyHUD>(PlayerController->GetHUD()))
			{
				LobbyUI = HUD->GetLobbyUI();
				LobbyUI->SetupPlayerWidgets();
				LobbyUI->BindWidgets();
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, "No hud found");
			}
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

void ALobbyPlayerPawn::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerPawn, LobbyUI);
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

void ALobbyPlayerPawn::ConfirmSelection()
{
	if(!LobbyUI)
	{
		GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, "No hud LobbyUI");
	}

	GEngine->AddOnScreenDebugMessage(-1, 50, FColor::Purple, "Confirmed 1");
	LobbyUI->ConfirmSelection();
		
	bIsSelectionReady = LobbyUI->CanBeConfirmed();
}

void ALobbyPlayerPawn::ReturnFromSelection()
{
	if(LobbyUI)
	{
		LobbyUI->ReturnFromSelection();
		
		bIsSelectionReady = false;
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
			State->SetCharacterDataAsset(Data);
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

