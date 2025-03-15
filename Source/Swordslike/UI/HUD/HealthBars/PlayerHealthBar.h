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
	
	void SetHealthBarSize(float MaxHealth);
	void SetStaminaBarSize(float MaxStamina);
	
	void SetHealthBarValue(float CurrentHealth, float MaxHealth);
	void SetStaminaBarValue(float CurrentStamina, float MaxStamina);
	void SetPostureBarValue(float Current, float Max);

	
	/**
	 * Binds the character's health amount to the HUD health bar.
	 * @param Character The character controller that has a BaseHealthComponent or a derived class from it.
	 */
	void BindHealthBar(ACharacter* Character);
	void BindStaminaBar(ACharacter* Character);
	void BindPostureBar(ACharacter* Character);

protected:
	virtual  void NativeConstruct() override;

private:
	const float PixelsPerHealthPoint = 4.f;
	const float PixelsPerStaminaPoint = 32.f;
};
