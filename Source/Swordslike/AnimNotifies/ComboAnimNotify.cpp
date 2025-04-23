#include "ComboAnimNotify.h"

#include "BaseCombatComponent.h"
#include "Common/WeaponHandlerComponent.h"
#include "Player/PlayerCombatComponent.h"
#include "Player/SwordslikeCharacter.h"


void UComboAnimNotify::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
	{
		return;
	}
	
	AActor* Owner = MeshComp->GetOwner();

	if (!Owner)
	{
		return;
	}

	const ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Owner);
	
	if(!CustomCharacter || !CustomCharacter->IsLocallyControlled())
	{
		return;
	}

	UPlayerCombatComponent* CombatComp = CustomCharacter->GetCombatComponent();
	
	if (!CombatComp)
	{
		return;
	}
	
	if(CombatComp->GetComboState() == EComboState::Broken)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "Broken OnNotify");
		return;
	}

	switch (NotifyType)
	{
		case ECombatNotifyType::Damage:
		{
			if (CombatComp->GetWeaponHandler())
			{
				CombatComp->GetWeaponHandler()->StartWeaponAttackDetection(HitType, Damage, CameraShake);
			}
			
			break;
		}
		case ECombatNotifyType::Input:
		{
			CombatComp->AllowInput();
			break;
		}
		case ECombatNotifyType::PerformNextAttack:
		{
			CombatComp->PerformNextAttack();
			break;
		}
		case ECombatNotifyType::IndicatorTime:
		{
			CombatComp->StartAttackWarning(TotalDuration, AnticipationSpeedMultiplayer);
			break;
		}
	}
}

void UComboAnimNotify::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
	{
		return;
	}
	
	AActor* Owner = MeshComp->GetOwner();

	if (!Owner)
	{
		return;
	}

	const ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Owner);
	
	if(!CustomCharacter || !CustomCharacter->IsLocallyControlled())
	{
		return;
	}

	UPlayerCombatComponent* CombatComp = CustomCharacter->GetCombatComponent();
	
	if (!CombatComp)
	{
		return;
	}
	
	if(CombatComp->GetComboState() == EComboState::Broken)
	{
		return;
	}
	
	switch (NotifyType)
	{
	case ECombatNotifyType::Damage:
		{
			if (CombatComp->GetWeaponHandler())
			{
				CombatComp->GetWeaponHandler()->StopWeaponAttackDetection();
			}
			break;
		}
	case ECombatNotifyType::Input:
		{
			CombatComp->DisableInput();
			break;
		}
	case ECombatNotifyType::IndicatorTime:
		{
			CombatComp->EndAttackWarning();
			break;
		}
	}
}

FString UComboAnimNotify::GetNotifyName_Implementation() const
{
	return NotifyName.IsNone() ? Super::GetNotifyName_Implementation() : NotifyName.ToString();
}