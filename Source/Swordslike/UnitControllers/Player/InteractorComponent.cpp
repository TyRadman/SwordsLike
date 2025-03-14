#include "InteractorComponent.h"

// TODO: remove this
AInteractorComponent::AInteractorComponent()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AInteractorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractorComponent::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

