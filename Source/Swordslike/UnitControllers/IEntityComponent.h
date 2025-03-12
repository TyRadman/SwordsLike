#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IEntityComponent.generated.h"

/**
 * 
 */
 UINTERFACE(Blueprintable)
class SWORDSLIKE_API UIEntityComponent : public UInterface
{
    GENERATED_BODY()
};

class SWORDSLIKE_API IIEntityComponent
{
    GENERATED_BODY()

public:
    virtual void InitEntityComponent(ACharacter* Character) = 0;
};