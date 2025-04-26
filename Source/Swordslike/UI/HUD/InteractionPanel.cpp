#include "InteractionPanel.h"

#include "WeaponComparisonWidget.h"
#include "Common/WeaponHandlerComponent.h"
#include "Components/TextBlock.h"
#include "Player/InteractionComponent.h"
#include "Player/SwordslikeCharacter.h"
#include "Swordslike/Combat/Interactable.h"

void UInteractionPanel::NativeConstruct()
{
	Super::NativeConstruct();

	HideInteractionPanel();
}

void UInteractionPanel::DisplayInteractionPanel(ASwordslikeCharacter* Character)
{
	if(!Character)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Character is missing")));
		return;
	}
	if(!Character->GetInteractionComponent())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("GetOverInteractionComponent is missing")));
		return;
	}
	if(!Character->GetInteractionComponent()->GetCurrentInteractable())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("CurrentInteractable is missing")));
		return;
	}

	if(IInteractable* Interactable = Character->GetInteractionComponent()->GetCurrentInteractable())
	{
		const FString Message = FString::Printf(TEXT("%s [%s]"), *Interactable->GetInteractionMessage(), *Character->GetInteractionInput());
		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("%s"), *Message));
		MessageText->SetText(FText::FromString(*Message));
		PlayAnimation(FadeInAnimation);

		if(AWeapon* NewWeapon = Cast<AWeapon>(Character->GetInteractionComponent()->GetCurrentInteractable()))
		{
			if(Character->GetWeaponHandler()->HasWeapon())
			{
				if(GetWorld()->GetTimerManager().IsTimerActive(ComparisonDisplayTimer))
				{
					GetWorld()->GetTimerManager().ClearTimer(ComparisonDisplayTimer);
				}
				
				AWeapon* CurrentWeapon = Character->GetWeaponHandler()->GetCurrentWeapon();
				AWeapon* InteractableWeapon = NewWeapon;

				GetWorld()->GetTimerManager().SetTimer(
					ComparisonDisplayTimer,
					[this, CurrentWeapon, InteractableWeapon]()
					{
						CurrentWeaponWidget->SetValues(CurrentWeapon, InteractableWeapon, false);
						NewWeaponWidget->SetValues(InteractableWeapon, CurrentWeapon, true);
						PlayAnimation(ComparisonFadeInAnimation);
						bIsComparingWeapons = true;
					},
					ComparisonDisplayDelay,
					false
				);
			}
		}
	}
}

void UInteractionPanel::HideInteractionPanel()
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("hidden"));
	PlayAnimation(FadeOutAnimation);
	
	if(GetWorld()->GetTimerManager().IsTimerActive(ComparisonDisplayTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(ComparisonDisplayTimer);
	}

	if(bIsComparingWeapons)
	{
		PlayAnimation(ComparisonFadeOutAnimation);
		bIsComparingWeapons = false;
	}
}
