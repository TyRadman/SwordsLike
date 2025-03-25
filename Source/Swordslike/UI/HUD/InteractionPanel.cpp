#include "InteractionPanel.h"

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
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Interaction HUD displaying UI")));
	
	if(!Character)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("Character is missing")));
		return;
	}
	if(!Character->GetOverInteractionComponent())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("GetOverInteractionComponent is missing")));
		return;
	}
	if(!Character->GetOverInteractionComponent()->GetCurrentInteractable())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("CurrentInteractable is missing")));
		return;
	}

	if(IInteractable* Interactable = Character->GetOverInteractionComponent()->GetCurrentInteractable())
	{
		FString Message = FString::Printf(TEXT("%s [%s]"), *Interactable->GetInteractionMessage(), *Character->GetInteractionInput());

		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, FString::Printf(TEXT("%s"), *Message));
		MessageText->SetText(FText::FromString(*Message));
		PlayAnimation(FadeInAnimation);
	}
}

void UInteractionPanel::HideInteractionPanel()
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("hidden"));
	PlayAnimation(FadeOutAnimation);
}
