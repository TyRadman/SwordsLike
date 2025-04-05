#include "MainPlayerState.h"

#include "Net/UnrealNetwork.h"
#include "Swordslike/SwordslikeGameInstance.h"

void AMainPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainPlayerState, CharacterDataAsset);
	DOREPLIFETIME(AMainPlayerState, PlayerPhase);
}

void AMainPlayerState::BeginPlay()
{
	Super::BeginPlay();
}

void AMainPlayerState::OnRep_PlayerPhase()
{
	
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
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Purple, "CALLED");
	
	if(OnCharacterChangedEvent.IsBound())
	{
		OnCharacterChangedEvent.Broadcast();
	}
}
