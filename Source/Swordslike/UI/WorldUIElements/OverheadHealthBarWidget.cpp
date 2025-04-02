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
				const FString Name = FString::Printf(TEXT("%s\n%s"),
				                                     *UEnum::GetValueAsString(Character->GetLocalRole()),
				                                     *UEnum::GetValueAsString(Character->GetRemoteRole()));
			
				SetOverheadNameValue(FText::FromString(*Name));
				// Hide();
			}
		}
		else
		{
			// Hide();
		}
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
	else
	{
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("%f\n%f"), Current, Max));
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
