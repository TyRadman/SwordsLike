#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Blueprint/UserWidget.h"
#include "MasterHUD.generated.h"

class UPlayerHealthBar;
class UInteractionPanel;

/**
 * The master HUD that holds all on-screen widget elements.
 */
UCLASS()
class SWORDSLIKE_API UMasterHUD : public UUserWidget, public IIEntityComponent
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UInteractionPanel* InteractionPanel;
	
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UPlayerHealthBar* PlayerStatsHUD;

public:
	virtual void InitEntityComponent(ACharacter* Character) override;
	
	FORCEINLINE UInteractionPanel* GetInteractionPanel() const { return InteractionPanel; } 
	FORCEINLINE UPlayerHealthBar* GetStatsHUD() const { return PlayerStatsHUD; } 
};
