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
			if(const ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character))
			{
				if(const UPlayerStartCharacterDataAsset* Data = PlayerCharacter->GetData())
				{
					GetStatsHUD()->SetHealthBarSize(Data->StartingHealthPoints);
					GetStatsHUD()->SetStaminaBarSize(Data->StartingStamina);
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "No data");
				}
			}
		}
	}
}
