#include "MainPlayerState.h"

#include "Net/UnrealNetwork.h"

void AMainPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainPlayerState, CharacterDataAsset);
}

void AMainPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);

	if (AMainPlayerState* PS = Cast<AMainPlayerState>(PlayerState))
	{
		if(CharacterDataAsset)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, TEXT("Copied Data"));
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

	if(OnPlayerNameChanged.IsBound())
	{
		OnPlayerNameChanged.Broadcast(GetPlayerName());
	}
}

void AMainPlayerState::SetPlayerNewName(const FString& NewName)
{
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
