#include "MainPlayerState.h"

#include "LobbyPlayerPawn.h"
#include "Net/UnrealNetwork.h"
#include "Swordslike/SwordslikeGameInstance.h"

void AMainPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainPlayerState, CharacterDataAsset);
	DOREPLIFETIME(AMainPlayerState, StateIndex);
}

void AMainPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AMainPlayerState* PS = Cast<AMainPlayerState>(PlayerState))
	{
		if(CharacterDataAsset)
		{
			// GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, TEXT("Copied Data"));
			PS->CharacterDataAsset = CharacterDataAsset;
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Black, TEXT("No Data to copy"));
		}
	}
}

void AMainPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();

	// to get rid of the default long name and set a new default
	if(!bIsDefaultNameSet)
	{
		bIsDefaultNameSet = true;
		
	TArray<FString> SoulNames = {
		TEXT("Aelric"), TEXT("Nyvara"), TEXT("Thorne"), TEXT("Kaelen"), TEXT("Veyla"),
		TEXT("Draven"), TEXT("Maelis"), TEXT("Riven"), TEXT("Zareth"), TEXT("Sorin"),
		TEXT("Vaelis"), TEXT("Noctis"), TEXT("Eryndor"), TEXT("Malrik"), TEXT("Saelyn"),
		TEXT("Kareth"), TEXT("Lyric"), TEXT("Vorn"), TEXT("Zalen"), TEXT("Orwyn")
	};

		const FString RandomSoulName = SoulNames[FMath::RandRange(0, SoulNames.Num() - 1)];
		SetPlayerName(RandomSoulName);
		ForceNetUpdate();
		return;
	}
	
	if(OnPlayerNameChanged.IsBound())
	{
		OnPlayerNameChanged.Broadcast(GetPlayerName());
	}
}

// Called on UMainLobbyMenu when the text widget changes. Only local calls
void AMainPlayerState::SetPlayerNewName(const FString& NewName)
{
	if(USwordslikeGameInstance* Instance = Cast<USwordslikeGameInstance>(GetGameInstance()))
	{
		Instance->PlayerName = NewName;
	}
	
	if(!HasAuthority())
	{
		Server_SetPlayerNewName(NewName);
	}
	else
	{
		PerformSetPlayerNewName(NewName);
	}
}

void AMainPlayerState::Server_SetPlayerNewName_Implementation(const FString& NewName)
{
	PerformSetPlayerNewName(NewName);
}

void AMainPlayerState::PerformSetPlayerNewName(const FString& NewName)
{
	SetPlayerName(NewName);
}

void AMainPlayerState::OnRep_CharacterDataAsset()
{
	if(OnCharacterChangedEvent.IsBound())
	{
		OnCharacterChangedEvent.Broadcast();
	}
}

void AMainPlayerState::OnRep_StateIndex()
{
	
}
