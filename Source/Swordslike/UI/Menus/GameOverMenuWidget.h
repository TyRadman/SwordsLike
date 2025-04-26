#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverMenuWidget.generated.h"

/**
 * The menu displayed when the player loses the game.
 */
UCLASS()
class SWORDSLIKE_API UGameOverMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void DisplayMenu();

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* DisplayMenuAnimation;
};
