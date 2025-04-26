#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WeaponComparisonWidget.generated.h"

class UImage;
class UTextBlock;
class AWeapon;
/**
 * 
 */
UCLASS()
class SWORDSLIKE_API UWeaponComparisonWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* WeaponNameText;
	
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* DamageText;
	
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* PostureDamageText;
	
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* StaminaCostText;
	
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UImage* WeaponIconImage;

	void SetValues(AWeapon* WeaponToSet, AWeapon* WeaponToCompareTo, const bool bSetValueColors);
};
