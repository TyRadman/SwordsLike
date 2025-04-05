#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "UtilHelper.generated.h"

class UWidget;
/**
 * Contains Unreal C++ helper methods and values that can be used across the project.
 */
UCLASS()
class SWORDSLIKE_API UUtilHelper : public UObject
{
	GENERATED_BODY()

public:
	static void ShowCursor(const UWorld* World, UWidget* Widget, bool bOnlyCursorControls = true);
	static void HideCursor(const UWorld* World);
};
