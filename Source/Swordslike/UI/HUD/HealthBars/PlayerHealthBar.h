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
	
	void SetHealthBarSize(const float MaxHealth);
	void SetStaminaBarSize(const float MaxStamina);
	
	void SetHealthBarValue(const float CurrentHealth, const float MaxHealth);
	void SetStaminaBarValue(const float CurrentStamina, const float MaxStamina);
	void SetPostureBarValue(const float Current, const float Max);

protected:
	virtual  void NativeConstruct() override;

private:
	const float PixelsPerHealthPoint = 4.f;
	const float PixelsPerStaminaPoint = 32.f;
};
