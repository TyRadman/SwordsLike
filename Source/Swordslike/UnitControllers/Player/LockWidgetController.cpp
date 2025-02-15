// Fill out your copyright notice in the Description page of Project Settings.


#include "LockWidgetController.h"

// Sets default values for this component's properties
ULockWidgetController::ULockWidgetController()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void ULockWidgetController::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();

	LockOnIndicatorInstance = GetWorld()->SpawnActor<AActor>(LockOnIndicatorClass, SpawnParams);
}


// Called every frame
void ULockWidgetController::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void ULockWidgetController::LockOnTarget(AActor* Target)
{
	if(LockOnIndicatorInstance)
	{
		LockOnIndicatorInstance->SetActorHiddenInGame(false);
		
		FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
		LockOnIndicatorInstance->AttachToActor(Target, AttachmentRules);
		
		FVector AttachmentPosition = Target->GetActorLocation();
		LockOnIndicatorInstance->SetActorLocation(AttachmentPosition);

	}
}

void ULockWidgetController::UnlockFromTarget()
{
	LockOnIndicatorInstance->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	LockOnIndicatorInstance->SetActorHiddenInGame(true);
}

