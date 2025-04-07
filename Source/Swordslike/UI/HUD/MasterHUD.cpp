#include "MasterHUD.h"

#include "BaseEntityData.h"
#include "GameFramework/Character.h"
#include "HealthBars/PlayerHealthBar.h"
#include "Player/MainPlayerState.h"
#include "Player/PlayerStartCharacterDataAsset.h"
#include "Player/SwordslikeCharacter.h"

void UMasterHUD::NativeConstruct()
{
	Super::NativeConstruct();

	
}

void UMasterHUD::InitEntityComponent(ACharacter* Character)
{
	if(Character)
	{
		if(Character->IsLocallyControlled())
		{
			if(const AMainPlayerState* PS = Cast<AMainPlayerState>(Character->GetPlayerState()))
			{
				if(const UPlayerStartCharacterDataAsset* Data = PS->GetCurrentDataAsset())
				{
					GetStatsHUD()->SetHealthBarSize(Data->StartingHealthPoints);
					GetStatsHUD()->SetStaminaBarSize(Data->StartingStamina);
				}
			}
		}
	}
}
