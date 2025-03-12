#include "TargetLockerComponent.h"
#include "LockWidgetController.h"
#include "SwordslikeCharacter.h"
#include "Camera/CameraComponent.h"
#include "Common/BaseHealthComponent.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "Swordslike/UnitControllers/Common/LockableTargetComponent.h"

UTargetLockerComponent::UTargetLockerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTargetLockerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// TODO: add this to notes
	DOREPLIFETIME(UTargetLockerComponent, bIsLockedOnTarget);
	DOREPLIFETIME(UTargetLockerComponent, LockedTarget);
}

void UTargetLockerComponent::InitEntityComponent(ACharacter* Character)
{
	ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character);
	
	if (PlayerCharacter)
	{
		if(PlayerCharacter->GetFollowCamera())
		{
			Camera = PlayerCharacter->GetFollowCamera();
		}
		else
		{
			PrintOnScreen(TEXT("No Camera Found!"));
		}
	
		if(PlayerCharacter->GetLockOnWidget())
		{
			LockIndicatorWidget = PlayerCharacter->GetLockOnWidget();
		}
		else
		{
			PrintOnScreen_Local(TEXT("No LockIndicatorWidget Found!"));
		}
	}
	else
	{
		PrintOnScreen(TEXT("No Player Controller Found!"));
	}
}

void UTargetLockerComponent::BeginPlay()
{
	Super::BeginPlay();

	if(APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		CharacterController = OwnerPawn->GetController();
	}
	else
	{
		PrintOnScreen(TEXT("No CharacterController Found!"));
	}
	
	// Cache the spring arm
	SpringArm = GetOwner()->FindComponentByClass<USpringArmComponent>();
}

#pragma region Tick
void UTargetLockerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(CanPerformLock())
	{
		ValidateLock();
		UpdateTargetLocation(DeltaTime);
	}
}

void UTargetLockerComponent::UpdateTargetLocation(float DeltaTime)
{
	FVector TargetLocation = LockedTarget->GetComponentLocation();
	FVector PlayerLocation = CharacterController->GetPawn()->GetActorLocation();
		
	FRotator TargetRotation = (TargetLocation - PlayerLocation).Rotation();

	FRotator NewRotation = CharacterController->GetControlRotation();
	NewRotation.Yaw = TargetRotation.Yaw;

	float InterpolationSpeed = 40.0f;
	FRotator FinalRotation = FMath::RInterpTo(CharacterController->GetControlRotation(),
		NewRotation, DeltaTime, InterpolationSpeed);
		
	CharacterController->SetControlRotation(FinalRotation);
}

void UTargetLockerComponent::ValidateLock()
{
	// if the target is not in range, start a timer
	if(!IsTargetInRange(LockedTarget))
	{
		if(!bIsTimerRunning)
		{
			bIsTimerRunning = true;
		
			GetOwner()->GetWorldTimerManager().SetTimer(
				OutOfSightTimer,
				[this](){bIsTimerFinished = true;},
				OffSightLockDuration,
				false);
		}
	}
	else
	{
		if(bIsTimerRunning)
		{
			GetOwner()->GetWorldTimerManager().ClearTimer(OutOfSightTimer);
			bIsTimerRunning = false;
		}
	}
	
	// if target is dead or if the timer for when the target went out of sight runs off
	if(!LockedTargetHealth->IsAlive() || bIsTimerFinished)
	{
		bIsTimerFinished = false;
		Unlock();
	}
}
#pragma endregion

// INPUT METHOD
void UTargetLockerComponent::PerformLockAction()
{
	if(bIsLockedOnTarget && LockedTarget)
	{
		Unlock();
		return;
	}

	LockOn();
}

#pragma region Unlock Server
void UTargetLockerComponent::Unlock()
{
	if(GetOwnerRole() < ROLE_Authority)
	{
		Server_Unlock();
	}
	else
	{
		Multicast_Unlock();
	}
}

void UTargetLockerComponent::Server_Unlock_Implementation()
{
	// PrintOnScreen_Local(TEXT("SERVER: There is a target"), 10.f);
	Multicast_Unlock();
}

// REPLICATED METHODS
void UTargetLockerComponent::Multicast_Unlock_Implementation()
{
	// PrintOnScreen_Local(TEXT("Multicast: There is a target"), 10.f);
	OnUnlockedTarget();
}

void UTargetLockerComponent::OnUnlockedTarget()
{
	bIsLockedOnTarget = false;

	if(LockedTarget)
	{
		// PrintOnScreen_Local(TEXT("There is a target"), 10.f);
		LockedTarget->OnUnlocked();
	}
	else
	{
		PrintOnScreen_Local(TEXT("How on earth is there no target"), 10.f);
	}
	
	LockedTarget = nullptr;

	LockIndicatorWidget->UnlockFromTarget();
	OnLockStateChanged.Broadcast(bIsLockedOnTarget);
}
#pragma endregion 


#pragma region Utilities
ULockableTargetComponent* UTargetLockerComponent::FindTarget()
{
	if (!GetOwner() || !SpringArm)
	{
		return nullptr;
	}

	FVector PlayerLocation = GetOwner()->GetActorLocation();
	FRotator CameraRotation = GetOwner()->GetInstigatorController()->GetControlRotation();
	FVector ForwardVector = CameraRotation.Vector();
	TArray<ULockableTargetComponent*> ValidTargets;
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	TArray<FOverlapResult> Overlaps;
	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		PlayerLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(SearchRadius),
		QueryParams
	);
	
	// filter valid targets
	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* Target = Overlap.GetActor();

		if(!Target)
		{
			continue;
		}
		
		if (ULockableTargetComponent* Lockable = Target->GetComponentByClass<ULockableTargetComponent>())
		{
			FVector DirectionToTarget = (Lockable->GetComponentLocation() - PlayerLocation).GetSafeNormal();
			float DotProduct = FVector::DotProduct(ForwardVector, DirectionToTarget);

			// TODO: should have this be a variable
			if (DotProduct > 0.5f && IsTargetInRange(Lockable) && Lockable->IsValidTarget()) 
			{
				ValidTargets.Add(Lockable);
			}
		}
	}

	if(ValidTargets.Num() > 0)
	{
		// sort by closest distance
		ValidTargets.Sort([PlayerLocation](USceneComponent& A, USceneComponent& B) {
			return FVector::DistSquared(PlayerLocation, A.GetComponentLocation()) < FVector::DistSquared(PlayerLocation, B.GetComponentLocation());
		});
		
		return ValidTargets[0];
	}

	return nullptr;
}

bool UTargetLockerComponent::CanPerformLock() const
{
	if(!LockedTarget)
	{
		return false;
	}
	
	if(!CharacterController)
	{
		return false;
	}

	
	if(!LockedTarget)
	{
		return false;
	}

	return true;
}

bool UTargetLockerComponent::IsTargetInRange(ULockableTargetComponent* Target) const
{
	FVector StartLocation = Camera->GetComponentLocation();
	FVector EndLocation = Target->GetComponentLocation();
	
	if(FVector::Dist(StartLocation, EndLocation) > DisconnectRadius)
	{
		return false;
	}
	
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	
	bool IsTargetHit = GetOwner()->GetWorld()->LineTraceSingleByChannel(
		HitResult, 
		StartLocation,
		EndLocation,
		ECC_Pawn,
		QueryParams
		);
	
	if(IsTargetHit && HitResult.GetActor() == Target->GetOwner())
	{
		// DrawDebugLine(GetOwner()->GetWorld(), StartLocation, EndLocation, FColor::Yellow, false, 5.f);
		return true;
	}
	
	// DrawDebugLine(GetOwner()->GetWorld(), StartLocation, EndLocation, FColor::Red, false, 5.f);
	return false;
}
#pragma endregion

#pragma region Lock Server
void UTargetLockerComponent::LockOn()
{
	ULockableTargetComponent* Target = FindTarget();

	if(Target)
	{
		PrintOnScreen_Local(TEXT("A TARGET IS IN SIGHT"), FColor::Green);
		Target->OnLocked();
		
		if(UBaseHealthComponent* Health = Target->GetOwner()->GetComponentByClass<UBaseHealthComponent>())
		{
			LockedTargetHealth = Health;
		}
		
		bIsLockedOnTarget = true;
		LockedTarget = Target;
		OnLockedTarget();
		
		if(GetOwnerRole() < ROLE_Authority)
		{
			Server_LockOn(Target);
		}
		else
		{
			bIsLockedOnTarget = true;
			LockedTarget = Target;
			OnLockedTarget();
			// Multicast_LockOn(Target);
		}
	}
	else
	{
		PrintOnScreen_Local(TEXT("NO TARGET IN SIGHT"));
	}
}

void UTargetLockerComponent::Server_LockOn_Implementation(ULockableTargetComponent* Target)
{
	if(Target)
	{
		bIsLockedOnTarget = true;
		LockedTarget = Target;
		Multicast_LockOn(Target);
	}
	else
	{
		PrintOnScreen_Local(TEXT("Server_LockOn_Implementation: No Target"));
	}
}

void UTargetLockerComponent::Multicast_LockOn_Implementation(ULockableTargetComponent* Target)
{
	if(IsAutonomousProxy())
	{
		return;
	}
	
	if(Target)
	{
		bIsLockedOnTarget = true;
		LockedTarget = Target;
		OnLockedTarget();
	}
	else
	{
		PrintOnScreen_Local(TEXT("Server_LockOn_Implementation: No Target"));
	}
}

void UTargetLockerComponent::OnLockedTarget()
{
	LockIndicatorWidget->LockOnTarget(LockedTarget);
	OnLockStateChanged.Broadcast(bIsLockedOnTarget);
}
#pragma endregion  

void UTargetLockerComponent::OnRep_IsLockedOn()
{
	OnLockStateChanged.Broadcast(bIsLockedOnTarget);
}

#pragma region Externals
void UTargetLockerComponent::SetLockIndicatorWidget(TObjectPtr<ULockWidgetController> LockIndicatorWidgetReference)
{
	LockIndicatorWidget = LockIndicatorWidgetReference;
}
#pragma endregion 