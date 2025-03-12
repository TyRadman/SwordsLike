// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NetworkOverheadDebugger.generated.h"

class UTextBlock;

/**
 * Debugs the character's name and network role
 */
UCLASS()
class SWORDSLIKE_API UNetworkOverheadDebugger : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* DebugText;

	void Setup(APawn* InParent);
};
