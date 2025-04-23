#include "WeaponComparisonWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Weapons/Weapon.h"

void UWeaponComparisonWidget::SetValues(AWeapon* WeaponToSet, AWeapon* WeaponToCompareTo)
{
	if(!WeaponToSet || !WeaponToCompareTo)
	{
		UE_LOG(LogTemp, Log, TEXT("No Weapon"));
		return;
	}

	WeaponNameText->SetText(FText::FromString(WeaponToSet->WeaponName));
	DamageText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), WeaponToSet->DamagePerHit)));
	PostureDamageText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), WeaponToSet->PostureDamagePerHit)));
	StaminaCostText->SetText(FText::FromString(FString::Printf(TEXT("%.1f"), WeaponToSet->StaminaPerHit)));

	if(WeaponToSet->WeaponIcon)
	{
		WeaponIconImage->SetBrushFromTexture(WeaponToSet->WeaponIcon);
	}
	
	if(WeaponToSet->DamagePerHit != WeaponToCompareTo->DamagePerHit)
	{
		if(WeaponToSet->DamagePerHit > WeaponToCompareTo->DamagePerHit)
		{
			DamageText->SetColorAndOpacity(FLinearColor::Green);
		}
		else
		{
			DamageText->SetColorAndOpacity(FLinearColor::Red);
		}
	}
	
	if(WeaponToSet->PostureDamagePerHit != WeaponToCompareTo->PostureDamagePerHit)
	{
		if(WeaponToSet->PostureDamagePerHit > WeaponToCompareTo->PostureDamagePerHit)
		{
			PostureDamageText->SetColorAndOpacity(FLinearColor::Green);
		}
		else
		{
			PostureDamageText->SetColorAndOpacity(FLinearColor::Red);
		}
	}

	
	if(WeaponToSet->StaminaPerHit != WeaponToCompareTo->StaminaPerHit)
	{
		if(WeaponToSet->StaminaPerHit < WeaponToCompareTo->StaminaPerHit)
		{
			StaminaCostText->SetColorAndOpacity(FLinearColor::Green);
		}
		else
		{
			StaminaCostText->SetColorAndOpacity(FLinearColor::Red);
		}
	}
}
