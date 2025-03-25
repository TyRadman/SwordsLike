#include "MasterHUD.h"

#include "BaseEntityData.h"
#include "GameFramework/Character.h"
#include "HealthBars/PlayerHealthBar.h"
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
			if(ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character))
			{
				GetStatsHUD()->SetHealthBarSize(PlayerCharacter->GetPlayerStats()->MaxHealthPoints);
				GetStatsHUD()->SetStaminaBarSize(PlayerCharacter->GetPlayerStats()->MaxStamina);
				GetStatsHUD()->BindStaminaBar(PlayerCharacter);
				GetStatsHUD()->BindPostureBar(PlayerCharacter);
			}
		}
	}
}
