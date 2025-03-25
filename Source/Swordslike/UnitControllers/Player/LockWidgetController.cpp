#include "LockWidgetController.h"
#include "SwordslikeCharacter.h"
#include "GameFramework/Character.h"

ULockWidgetController::ULockWidgetController()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULockWidgetController::InitEntityComponent(ACharacter* Character)
{
	if(Character)
	{
		if(Character->IsLocallyControlled())
		{
			if(ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character))
			{
				// spawn indicator
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = GetOwner()->GetInstigator();

				if(LockOnIndicatorClass)
				{
					LockOnIndicatorInstance = GetWorld()->SpawnActor<AActor>(LockOnIndicatorClass, SpawnParams);
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, "No Class widget assigned!");
				}

				// hide indicator
				HideIndicator();
			}
		}
	}
}

void ULockWidgetController::ShowIndicatorOnTarget(USceneComponent* Target)
{
	if(!Target)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "No target for the lock on widget");
		return;
	}
	
	if (!LockOnIndicatorInstance)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "ULockWidgetController Lock: No instance of the widget");
		return;
	}

	LockOnIndicatorInstance->SetActorHiddenInGame(false);

	FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules::SnapToTargetNotIncludingScale;
	LockOnIndicatorInstance->AttachToComponent(Target, AttachmentRules);

	FVector AttachmentPosition = Target->GetComponentLocation();
	LockOnIndicatorInstance->SetActorLocation(AttachmentPosition);
}

void ULockWidgetController::HideIndicator()
{
	if(!LockOnIndicatorInstance)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, "ULockWidgetController Hide: No instance of the widget");
		return;
	}
	
	LockOnIndicatorInstance->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	LockOnIndicatorInstance->SetActorHiddenInGame(true);
}

bool ULockWidgetController::IsLocallyControlledActor() const
{
	return GetNetMode() == NM_Standalone || GetOwnerRole() == ROLE_AutonomousProxy;
}

