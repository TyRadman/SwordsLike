#include "OverheadHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"

void UOverheadHealthBarWidget::InitEntityComponent(ACharacter* Character)
{
	if(Character)
	{
		Hide();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 500.f, FColor::Green, FString::Printf(TEXT("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!Hidden the overhead widget")));
	}
}

void UOverheadHealthBarWidget::SetHealthOverheadBarValue(const float CurrentHealth, const float MaxHealth)
{
	if(HealthBar)
	{
		const float Amount = CurrentHealth / MaxHealth;
		HealthBar->SetPercent(Amount);
	}
}

void UOverheadHealthBarWidget::SetPostureOverheadBarValue(const float Current, const float Max)
{
	if(PostureBar)
	{
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("%f \\ %f"), Current, Max));
		const float Amount = Current / Max;
		PostureBar->SetPercent(Amount);
	}
}

void UOverheadHealthBarWidget::SetOverheadNameValue(const FText& NameText)
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

void UOverheadHealthBarWidget::NativeDestruct()
{
	RemoveFromParent();
	
	Super::NativeDestruct();
}
