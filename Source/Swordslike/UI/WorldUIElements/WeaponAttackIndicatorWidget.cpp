#include "WeaponAttackIndicatorWidget.h"
#include "Player/SwordslikeCharacter.h"

void UWeaponAttackIndicatorWidget::Shrink(const float Duration)
{
	Show();
	PlayAnimation(ShrinkAnimation, 0.f, 1, EUMGSequencePlayMode::Forward, 1.f / Duration);
}

void UWeaponAttackIndicatorWidget::InitEntityComponent(ACharacter* Character)
{
	if(Character)
	{
		if(ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character))
		{
			Hide();
		}
	}
}

void UWeaponAttackIndicatorWidget::Hide()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UWeaponAttackIndicatorWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);
}
