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

	// TODO: ToRemoveOnCook
	SetIsReplicatedByDefault(true);
}

void UTargetLockerComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTargetLockerComponent, bIsLockedOnTarget);
	DOREPLIFETIME(UTargetLockerComponent, LockedTarget);
	DOREPLIFETIME(UTargetLockerComponent, LockedTargetHealth);
}

void UTargetLockerComponent::InitEntityComponent(ACharacter* Character)
{
	if (ASwordslikeCharacter* PlayerCharacter = Cast<ASwordslikeCharacter>(Character))
	{
		if(!GetIsReplicated())
		{
			PrintOnScreen(TEXT("UTargetLockerComponent: NOT REPLICATED! SHOULD BE!"));
		}
		
		Server_OnLockStateChanged.AddUObject(PlayerCharacter, &ASwordslikeCharacter::OnTargetLockedOn);
		
		if(PlayerCharacter->GetFollowCamera())
		{
			Camera = PlayerCharacter->GetFollowCamera();
		}
		else
		{
			PrintOnScreen(TEXT("UTargetLockerComponent: No Camera Found!"));
		}
	
		if(PlayerCharacter->GetLockOnWidget())
		{
			LockIndicatorWidget = PlayerCharacter->GetLockOnWidget();
		}
		else
		{
			PrintOnScreen(TEXT("UTargetLockerComponent: No LockIndicatorWidget Found!"), FColor::Red, 20.f);
		}

		CharacterController = Character->GetController();
		SpringArm = PlayerCharacter->GetCameraBoom();
	}
	else
	{
		PrintOnScreen(TEXT("UTargetLockerComponent: No Player Controller Found!"));
	}
}

#pragma region Tick
void UTargetLockerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!IsLocallyControlled())
	{
		return;
	}

	if(CanPerformLock())
	{
		ValidateLock();
		UpdateTargetLocation(DeltaTime);
	}
	else if(bIsLockedOnTarget)
	{
		// PrintOnScreen(TEXT("Can't perform lock while locked!"));
	}
}

bool UTargetLockerComponent::CanPerformLock() const
{
	if(!bIsLockedOnTarget)
	{
		// PrintOnScreen_Local(TEXT("No bIsLockedOnTarget"));
		return false;
	}
	
	if(!LockedTarget)
	{
		PrintOnScreen_Local(TEXT("No LockedTarget"));
		return false;
	}
	
	if(!CharacterController)
	{
		PrintOnScreen_Local(TEXT("No CharacterController"));
		return false;
	}

	return true;
}

void UTargetLockerComponent::UpdateTargetLocation(const float DeltaTime)
{
	if(!LockedTarget)
	{
		return;
	}
	
	const FVector TargetLocation = LockedTarget->GetComponentLocation();
	const FVector PlayerLocation = CharacterController->GetPawn()->GetActorLocation();
	const FRotator TargetRotation = (TargetLocation - PlayerLocation).Rotation();
	FRotator NewRotation = CharacterController->GetControlRotation();
	
	NewRotation.Yaw = TargetRotation.Yaw;
	constexpr float InterpolationSpeed = 40.0f;
	const FRotator FinalRotation = FMath::RInterpTo(CharacterController->GetControlRotation(),
	                                                NewRotation, DeltaTime, InterpolationSpeed);
		
	CharacterController->SetControlRotation(FinalRotation);
}

/**
 * Validates whether the target can still be locked-on to by whether it's alive, with sight, and within range.
 */
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
		UE_LOG(LogTemp, Warning, TEXT("Unlocked"));
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
	if(LockedTarget)
	{
		LockedTarget->OnUnlocked();
	}
	else
	{
		PrintOnScreen_Local(FString::Printf(TEXT("No target on unlock")));
	}
	
	LockIndicatorWidget->HideIndicator();

	if(!HasAuthority())
	{
		Server_Unlock();
	}
	else
	{
		OnUnlockedTarget();
	}
}

void UTargetLockerComponent::Server_Unlock_Implementation()
{
	OnUnlockedTarget();
}

void UTargetLockerComponent::OnUnlockedTarget()
{
	bIsLockedOnTarget = false;
	LockedTarget = nullptr;

	Server_OnLockStateChanged.Broadcast(nullptr, false);
}
#pragma endregion 

#pragma region Lock Server
void UTargetLockerComponent::LockOn()
{
	if(ULockableTargetComponent* Target = FindTarget())
	{
		if(Local_OnLockStateChanged.IsBound())
		{
			Local_OnLockStateChanged.Broadcast(Target, true);
		}
		
		Target->OnLocked();
		LockIndicatorWidget->ShowIndicatorOnTarget(Target);
		
		if(!HasAuthority())
		{
			Server_LockOn(Target);
		}
		else
		{
			OnLockedTarget(Target);
		}
	}
	else
	{
		PrintOnScreen(TEXT("NO TARGET IN SIGHT"));
	}
}

void UTargetLockerComponent::Server_LockOn_Implementation(ULockableTargetComponent* Target)
{
	if(Target)
	{
		OnLockedTarget(Target);
	}
	else
	{
		PrintOnScreen(TEXT("Server_LockOn_Implementation: No Target"));
	}
}

void UTargetLockerComponent::OnLockedTarget(ULockableTargetComponent* Target)
{
	if(!Target)
	{
		PrintOnScreen(TEXT("OnLockedTarget: No Target"));
		return;
	}
	
	if(UBaseHealthComponent* Health = Target->GetOwner()->GetComponentByClass<UBaseHealthComponent>())
	{
		LockedTargetHealth = Health;
	}
	
	bIsLockedOnTarget = true;
	LockedTarget = Target;
	Server_OnLockStateChanged.Broadcast(Target, true);
}
#pragma endregion  

void UTargetLockerComponent::OnRep_bIsLockedOnTarget()
{
	if(!HasAuthority())
	{
		// if (bIsLockedOnTarget)
		// {
		// }
		// else
		// {
		// }
		
		// PrintOnScreen_Local(FString::Printf(TEXT("OnRep_IsLockedOn triggered, value: %s"),
		// 	bIsLockedOnTarget ? TEXT("true") : TEXT("false")), FColor::Green);
	}
	else
	{
		PrintOnScreen_Local(FString::Printf(TEXT("CHATGPT IS SPREADING MISINFORMATION")));
	}
}


#pragma region Utilities
ULockableTargetComponent* UTargetLockerComponent::FindTarget()
{
	if (!GetOwner() || !SpringArm)
	{
		return nullptr;
	}

	if(!CharacterController)
	{
		PrintOnScreen(TEXT("UTargetLockerComponent: No Player Controller Found!"));
		return nullptr;
	}

	FVector PlayerLocation = GetOwner()->GetActorLocation();
	FVector ForwardVector = CharacterController->GetControlRotation().Vector();
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

bool UTargetLockerComponent::IsTargetInRange(const ULockableTargetComponent* Target) const
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