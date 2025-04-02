#include "AttackEnd_AN.h"

#include "Player/SwordslikeCharacter.h"

void UAttackEnd_AN::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                           const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if(MeshComp && MeshComp->GetOwner())
	{
		if(const ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner()))
		{
			// GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Red, TEXT("UAttackEnd_AN SUCCESS"));
			// Character->GetCombatComponent()->OnAttackEnded(nullptr, false);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Red, TEXT("UAttackEnd_AN No ASwordslikeCharacter"));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0, FColor::Red, TEXT("UAttackEnd_AN No MeshComp"));
	}
}
