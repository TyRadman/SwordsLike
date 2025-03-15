#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MasterHUD.generated.h"

class UPlayerHealthBar;
class UInteractionPanel;

/**
 * The master HUD that holds all on-screen widget elements.
 */
UCLASS()
class SWORDSLIKE_API UMasterHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	
private:

	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UInteractionPanel* InteractionPanel;
	
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UPlayerHealthBar* PlayerStatsHUD;

public:
	FORCEINLINE UInteractionPanel* GetInteractionPanel() const { return InteractionPanel; } 
	FORCEINLINE UPlayerHealthBar* GetStatsHUD() const { return PlayerStatsHUD; } 
};
