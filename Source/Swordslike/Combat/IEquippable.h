// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IEquippable.generated.h"

UINTERFACE(Blueprintable)
class SWORDSLIKE_API UIEquippable : public UInterface
{
	GENERATED_BODY()
};

class SWORDSLIKE_API IIEquippable
{
	GENERATED_BODY()
	
public:
	void Equip();
	
};
