#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHealthBar.generated.h"

class USizeBox;
class UProgressBar;

/**
 * A UserWidget that displays the player's HP and Stamina (Name should be changed.) 
 */
UCLASS()
class SWORDSLIKE_API UPlayerHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	// So that we can bind this to the Progress Bar in your widget
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UProgressBar* HealthBar;
	
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UProgressBar* StaminaBar;
	
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UProgressBar* PostureBar;

	void SetHealthBarValue(float CurrentHealth, float MaxHealth);
	void SetStaminaBarValue(float CurrentStamina, float MaxStamina);
	void SetPostureBarValue(float Current, float Max);

protected:
	// Override the Initialize function to set up the widget
	virtual  void NativeConstruct() override;
};
