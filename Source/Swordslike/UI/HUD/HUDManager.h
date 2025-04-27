#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "HUDManager.generated.h"

class UGameOverMenuWidget;
class UMasterHUD;
class UPlayerHealthBar;
/**
 * The HUD that holds the stats of the player such as HP, Stamina, and Posture.
 */
UCLASS()
class SWORDSLIKE_API AHUDManager : public AHUD
{
	GENERATED_BODY()

public:
	AHUDManager();

	FORCEINLINE UMasterHUD* GetMasterHUD() const { return MasterHUD; }
	FORCEINLINE UGameOverMenuWidget* GetGameOverMenu() const { return GameOverMenu; }


protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UMasterHUD* MasterHUD;
	
	UPROPERTY(EditDefaultsOnly, Category = References, meta=(AllowPrivateAccess=true))
	TSubclassOf<UMasterHUD> MasterHUDReference;
	UPROPERTY(EditDefaultsOnly, Category = References, meta=(AllowPrivateAccess=true))
	TSubclassOf<UGameOverMenuWidget> GameOverMenuReference;
	UPROPERTY()
	UGameOverMenuWidget* GameOverMenu;

	void CreateHealthBar();
};
