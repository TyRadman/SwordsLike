#include "ActionValidationManager.h"

UActionValidationManager::UActionValidationManager()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UActionValidationManager::BeginPlay()
{
	Super::BeginPlay();
}

void UActionValidationManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

