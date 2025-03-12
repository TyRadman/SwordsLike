#include "LockWidgetController.h"

ULockWidgetController::ULockWidgetController()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULockWidgetController::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = GetOwner()->GetInstigator();
	// SpawnParams.Name = FName(*FString::Printf(TEXT("Lock Widget (%s) (%p)"), *UEnum::GetValueAsString(GetOwnerRole()), this));

	if(LockOnIndicatorClass)
	{
		LockOnIndicatorInstance = GetWorld()->SpawnActor<AActor>(LockOnIndicatorClass, SpawnParams);
		FString Name = FString::Printf(TEXT("Lock Widget (%s) (%p)"), *UEnum::GetValueAsString(GetOwnerRole()), this);
		PrintOnScreen_Local(FString::Printf(TEXT("Changed name to %s"), *Name));
		LockOnIndicatorInstance->Rename(*Name);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, "No Class widget assigned!");
	}
}

void ULockWidgetController::LockOnTarget(USceneComponent* Target)
{
	if(!Target)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "No target for the lock on widget");
		return;
	}
	
	if (IsLocallyControlledActor())
	{
		if (!LockOnIndicatorInstance)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "No instance of the widget");
			return;
		}

		LockOnIndicatorInstance->SetActorHiddenInGame(false);

		FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
		LockOnIndicatorInstance->AttachToComponent(Target, AttachmentRules);

		FVector AttachmentPosition = Target->GetComponentLocation();
		LockOnIndicatorInstance->SetActorLocation(AttachmentPosition);
	}
}

void ULockWidgetController::UnlockFromTarget()
{
	if(!LockOnIndicatorInstance)
	{
		return;
	}
	
	if(IsLocallyControlledActor())
	{
		HideIndicator();
	}
}

void ULockWidgetController::HideIndicator()
{
	if(LockOnIndicatorInstance)
	{
		LockOnIndicatorInstance->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		LockOnIndicatorInstance->SetActorHiddenInGame(true);
	}
}

bool ULockWidgetController::IsLocallyControlledActor() const
{
	return GetNetMode() == NM_Standalone || GetOwnerRole() == ROLE_AutonomousProxy;
}

