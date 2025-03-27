#include "ComboAnimNotify.h"

#include "BaseCombatComponent.h"
#include "Common/WeaponHandlerComponent.h"
#include "Player/PlayerCombatComponent.h"
#include "Player/SwordslikeCharacter.h"


void UComboAnimNotify::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (!MeshComp)
		return;

	AActor* Owner = MeshComp->GetOwner();

	if (!Owner)
	{
		return;
	}

	ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Owner);
	
	if(!CustomCharacter)
	{
		return;
	}

	UPlayerCombatComponent* CombatComp = CustomCharacter->GetCombatComponent();
	
	if (!CombatComp)
		return;

	switch (NotifyType)
	{
	case ECombatNotifyType::Damage:
		{
			if (CombatComp->GetWeaponHandler())
			{
				CombatComp->GetWeaponHandler()->StartWeaponAttackDetection();
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
			CombatComp->StartAttackWarning(TotalDuration);
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

	ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Owner);
	
	if(!CustomCharacter)
	{
		return;
	}

	UPlayerCombatComponent* CombatComp = CustomCharacter->GetCombatComponent();
	
	if (!CombatComp)
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