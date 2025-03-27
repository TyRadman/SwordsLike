#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Blueprint/UserWidget.h"
#include "WeaponAttackIndicatorWidget.generated.h"

/**
 * 
 */
UCLASS()
class SWORDSLIKE_API UWeaponAttackIndicatorWidget : public UUserWidget, public IIEntityComponent
{
	GENERATED_BODY()
	
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ShrinkAnimation;

	void Hide();
	void Show();

public:
	void Shrink(const float Duration);
	virtual void InitEntityComponent(ACharacter* Character) override;
};
