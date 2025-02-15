#include "TargetLockerComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "LockWidgetController.h"
#include "Engine/OverlapResult.h"
#include "Swordslike/UnitControllers/Common/LockableTargetComponent.h"

// Sets default values for this component's properties
UTargetLockerComponent::UTargetLockerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UTargetLockerComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Get the Player Controller
	APlayerController* PlayerController = Cast<APlayerController>(GetOwner()->GetInstigatorController());

	if (!PlayerController)
	{
		return;
	}
	// Get the Enhanced Input Subsystem
	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
	
	if (InputSubsystem && DefaultMappingContext)
	{
		// Add the Input Mapping Context
		InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	// Bind Actions
	if (UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerController->InputComponent))
	{
		EnhancedInputComp->BindAction(LockAction, ETriggerEvent::Completed, this, &UTargetLockerComponent::LockToTarget);
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());

	if(OwnerPawn)
	{
		CharacterController = OwnerPawn->GetController();
	}

	// Cache the spring arm
	SpringArm = GetOwner()->FindComponentByClass<USpringArmComponent>();
}


// Called every frame
void UTargetLockerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(LockedTarget)
	{
		if(!CharacterController)
		{
			return;
		}

		FVector TargetLocation = LockedTarget->GetActorLocation();
		FVector PlayerLocation = CharacterController->GetPawn()->GetActorLocation();
		
		FRotator TargetRotation = (TargetLocation - PlayerLocation).Rotation(); // Direction to target

		FRotator NewRotation = CharacterController->GetControlRotation(); // Get current control rotation
		NewRotation.Yaw = TargetRotation.Yaw; // Lock only Yaw to target

		float InterpolationSpeed = 40.0f;
		FRotator FinalRotation = FMath::RInterpTo(CharacterController->GetControlRotation(),
			NewRotation, DeltaTime, InterpolationSpeed);
		
		CharacterController->SetControlRotation(FinalRotation); // Apply new rotation

	}
}

void UTargetLockerComponent::SetLockIndicatorWidget(TObjectPtr<ULockWidgetController> LockIndicatorWidgetReference)
{
	LockIndicatorWidget = LockIndicatorWidgetReference;
}

void UTargetLockerComponent::LockToTarget()
{
	if (!GetOwner() || !SpringArm)
	{
		GEngine->AddOnScreenDebugMessage(0, 5, FColor::Red, FString::Printf(TEXT("Nulls")));
		UE_LOG(LogTemp, Error, TEXT("LockToTarget is NULL"));
		return;
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(0, 5, FColor::Red, FString::Printf(TEXT("No nulls")));
	}

	// when unlocking
	if(IsLockedOnTarget && LockedTarget)
	{
		IsLockedOnTarget = false;
		LockedTarget = nullptr;
		LockIndicatorWidget->UnlockFromTarget();
		return;
	}
	
	FVector PlayerLocation = GetOwner()->GetActorLocation();
	FRotator CameraRotation = GetOwner()->GetInstigatorController()->GetControlRotation();
	FVector ForwardVector = CameraRotation.Vector();

	DrawDebugLine(GetWorld(), PlayerLocation, PlayerLocation + ForwardVector * 1000.0f, FColor::Red);
    
	TArray<AActor*> PotentialTargets;
	TArray<AActor*> ValidTargets;
	
	float SearchRadius = 1000.0f;
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
		
		if (Target && Target->FindComponentByClass<ULockableTargetComponent>())
		{
			FVector DirectionToTarget = (Target->GetActorLocation() - PlayerLocation).GetSafeNormal();
			float DotProduct = FVector::DotProduct(ForwardVector, DirectionToTarget);

			// TODO: should have this be a variable
			if (DotProduct > 0.5f) 
			{
				ValidTargets.Add(Target);
			}
		}
	}

	if(ValidTargets.Num() > 0)
	{
		// sort by closest distance
		ValidTargets.Sort([PlayerLocation](AActor& A, AActor& B) {
			return FVector::DistSquared(PlayerLocation, A.GetActorLocation()) < FVector::DistSquared(PlayerLocation, B.GetActorLocation());
		});
	}
	
	// if there is a target then....
	if (ValidTargets.Num() > 0)
	{
		LockedTarget = ValidTargets[0];
		IsLockedOnTarget = true;

		LockIndicatorWidget->LockOnTarget(LockedTarget);

		if(LockedTarget != nullptr)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("Locked on Target: %s."), *LockedTarget->GetActorNameOrLabel()));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, FString::Printf(TEXT("Target is null for some reason")));
		}
	}
}
