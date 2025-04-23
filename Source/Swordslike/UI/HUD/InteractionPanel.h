#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPanel.generated.h"

class UWeaponComparisonWidget;
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
	
	UPROPERTY(Transient,  meta = (BindWidgetAnim))
	UWidgetAnimation* ComparisonFadeInAnimation;
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ComparisonFadeOutAnimation;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;
	
	UPROPERTY(meta = (BindWidget))
	UWeaponComparisonWidget* CurrentWeaponWidget;
	UPROPERTY(meta = (BindWidget))
	UWeaponComparisonWidget* NewWeaponWidget;

	virtual void NativeConstruct() override;

	bool bIsComparingWeapons = false;

	FTimerHandle ComparisonDisplayTimer;
	const float ComparisonDisplayDelay = 0.5f;
	
public:
	void DisplayInteractionPanel(ASwordslikeCharacter* Character);
	void HideInteractionPanel();
};
