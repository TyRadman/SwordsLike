#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Blueprint/UserWidget.h"
#include "OverheadHealthBarWidget.generated.h"

class UProgressBar;
class UTextBlock;

/**
 * Displays an overhead health bar.
 */
UCLASS()
class SWORDSLIKE_API UOverheadHealthBarWidget : public UUserWidget, public IIEntityComponent
{
	GENERATED_BODY()

public:
	virtual void InitEntityComponent(ACharacter* Character) override;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	UProgressBar* PostureBar;
	
	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameTextBlock;
	
	void SetHealthOverheadBarValue(const float CurrentHealth, const float MaxHealth);
	void SetPostureOverheadBarValue(const float Current, const float Max);
	void SetOverheadNameValue(const FText& NameText);

	void Show();
	void Hide();
	
protected:
	// Override the Initialize function to set up the widget
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};
