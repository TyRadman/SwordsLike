#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Swordslike/UnitControllers/BaseCombatComponent.h"
#include "PlayerCombatComponent.generated.h"

class UInputMappingContext;
class UInputAction;
class UWeaponHandlerComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API UPlayerCombatComponent : public UBaseCombatComponent
{
	GENERATED_BODY()

public:	
	UPlayerCombatComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Roll() override;
};
