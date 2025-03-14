#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ActionValidationManager.generated.h"

UENUM(BlueprintType)
enum class ActionType : uint8
{
	Attack = 0,
	Roll = 1,
	Jump = 2,
};

DECLARE_DELEGATE_RetVal(bool, BoolDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API UActionValidationManager : public UActorComponent
{
	GENERATED_BODY()

public:	
	UActionValidationManager();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	
		
};
