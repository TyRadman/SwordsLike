#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPanel.generated.h"

class UTextBlock;
class ASwordslikeCharacter;

/**
 * 
 */
UCLASS()
class SWORDSLIKE_API UInteractionPanel : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY(Transient,  meta = (BindWidgetAnim))
	UWidgetAnimation* FadeInAnimation;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* FadeOutAnimation;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;

	virtual void NativeConstruct() override;

public:
	void DisplayInteractionPanel(ASwordslikeCharacter* Character);
	void HideInteractionPanel();
};
