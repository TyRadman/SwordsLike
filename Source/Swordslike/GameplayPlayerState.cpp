#include "GameplayPlayerState.h"

#include "GameplayGameState.h"
#include "SwordslikeGameInstance.h"
#include "Player/SwordslikeCharacter.h"

void AGameplayPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (AGameplayGameState* GameState = GetWorld()->GetGameState<AGameplayGameState>())
	{
		GameState->OnPlayerDeathReported.AddLambda([this, GameState](APlayerState* DeadPlayer)
			{
				GameState->CheckAlivePlayers();
			});
	}

	if(const AController* Controller = GetOwner<AController>())
	{
		if (APawn* Pawn = Controller->GetPawn())
		{
			if (ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(Pawn))
			{
				PlayerCharacter = Character;
			}
		}
	}

	// FTimerHandle Timer;
	// GetWorld()->GetTimerManager().SetTimer(Timer,
	// 	[this]()
	// 	{
	// 		if (IsOwnedBy(GetWorld()->GetFirstPlayerController()))
	// 		{
	// 			if(const USwordslikeGameInstance* Instance = Cast<USwordslikeGameInstance>(GetGameInstance()))
	// 			{
	// 				UE_LOG(LogTemp, Warning, TEXT("Called to start %s"), *Instance->PlayerName);
	//
	// 				if(GetLocalRole() == ROLE_Authority)
	// 				{
	// 					SetPlayerName(Instance->PlayerName);
	// 				}
	// 				else
	// 				{
	// 					Server_SetPlayerName(Instance->PlayerName);
	// 				}
	// 			}
	// 		}
	// 	}, 5.0f, false);
}

void AGameplayPlayerState::OnRep_PlayerName()
{
	Super::OnRep_PlayerName();
	
	// if(const AController* Controller = GetOwner<AController>())
	// {
	// 	if (APawn* Pawn = Controller->GetPawn())
	// 	{
	// 		if (ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(Pawn))
	// 		{
	// 			PlayerCharacter = Character;
	//
	// 			PlayerCharacter->SetOverheadText(GetPlayerName());
	// 			UE_LOG(LogTemp, Warning, TEXT("The overhead name set to %s"), *GetPlayerName());
	// 		}
	// 	}
	// }
	// else
	// {
	// 	UE_LOG(LogTemp, Error, TEXT("No character controller found %s"), *GetPlayerName());
	// }
}

void AGameplayPlayerState::Server_SetPlayerName_Implementation(const FString& NameToSet)
{
	SetPlayerName(NameToSet);
}
