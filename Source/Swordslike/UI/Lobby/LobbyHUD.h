#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LobbyHUD.generated.h"

class UMainLobbyMenu;

/**
 * 
 */
UCLASS()
class SWORDSLIKE_API ALobbyHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	ALobbyHUD();
	UMainLobbyMenu* GetLobbyUI() const { return LobbyUI; }

	UPROPERTY(EditDefaultsOnly, Category = "References")
	TSubclassOf<UUserWidget> LobbyWidgetClass;


protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	UMainLobbyMenu* LobbyUI;
};
