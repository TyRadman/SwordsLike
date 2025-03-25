#include "OverheadHealthBarWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/Character.h"
#include "Player/SwordslikeCharacter.h"

void UOverheadHealthBarWidget::InitEntityComponent(ACharacter* Character)
{
	if(Character)
	{
		if(!Character->IsLocallyControlled())
		{
			if(ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character))
			{
				FString Name = FString::Printf(TEXT("%s\n%s"),
					*UEnum::GetValueAsString(Character->GetLocalRole()),
					*UEnum::GetValueAsString(Character->GetRemoteRole()));
			
				SetNameValue(FText::FromString(*Name));
				Hide();
			}
		}
		else
		{
			Hide();
		}
	}
}

void UOverheadHealthBarWidget::SetHealthBarValue(float CurrentHealth, float MaxHealth)
{
	if(HealthBar)
	{
		float amount = CurrentHealth / MaxHealth;
		HealthBar->SetPercent(amount);
	}
}

void UOverheadHealthBarWidget::SetPostureBarValue(float Current, float Max)
{
	if(PostureBar)
	{
		float amount = Current / Max;
		PostureBar->SetPercent(amount);
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

void UOverheadHealthBarWidget::NativeDestruct()
{
	RemoveFromParent();
	
	Super::NativeDestruct();
}
