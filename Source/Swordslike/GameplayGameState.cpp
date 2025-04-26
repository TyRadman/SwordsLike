#include "GameplayGameState.h"

#include "GameFramework/PlayerState.h"
#include "Player/SwordslikeCharacter.h"

void AGameplayGameState::ReportDeath(APlayerState* DeadPlayer)
{
	OnPlayerDeathReported.Broadcast(DeadPlayer);
}

void AGameplayGameState::CheckAlivePlayers()
{
	int32 AliveCount = 0;

	for (APlayerState* PS : PlayerArray)
	{
		if (!IsValid(PS))
		{
			continue;
		}
		
		if (APawn* Pawn = PS->GetPawn())
		{
			if(ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(Pawn))
			{
				if(Character->GetHealthComponent()->IsAlive())
				{
					++AliveCount;
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("%d Alive"), AliveCount);
}
