#include "OverheadHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UOverheadHealthBarWidget::SetHealthBarValue(float CurrentHealth, float MaxHealth)
{
	if(HealthBar)
	{
		float amount = CurrentHealth / MaxHealth;
		HealthBar->SetPercent(amount);
	}
}

void UOverheadHealthBarWidget::SetNameValue(FText NameText)
{
	if (NameTextBlock)
	{
		NameTextBlock->SetText(NameText);
	}
}

void UOverheadHealthBarWidget::Show()
{
	SetVisibility(ESlateVisibility::Visible);
}

void UOverheadHealthBarWidget::Hide()
{
	SetVisibility(ESlateVisibility::Hidden);
}

void UOverheadHealthBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
}
