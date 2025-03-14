#include "InteractionComponent.h"

#include "Swordslike/Combat/Interactable.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UInteractionComponent::Interact()
{
	PrintOnScreen_Local(TEXT("Interact 1"));
	
	if(!GetOwner()->HasAuthority())
	{
		PrintOnScreen_Local(TEXT("Interact 2"));
		Server_Interact();
	}
	else
	{
		Multicast_Interact();
	}
}

void UInteractionComponent::Server_Interact_Implementation()
{
	Multicast_Interact();
}

void UInteractionComponent::Multicast_Interact_Implementation()
{
	PrintOnScreen(TEXT("Interact 3"));
	FVector Start = GetOwner()->GetActorLocation();
	FVector End = Start;
	float Radius = 100.f;
	FHitResult HitResults;
	FCollisionQueryParams Params;

	Params.AddIgnoredActor(GetOwner());
	
	bool HasInteractable = GetWorld()->SweepSingleByObjectType(
		HitResults,
		Start,
		End,
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeSphere(Radius),
		Params
		);

	if(HasInteractable)
	{
		// if it's interactable
		if(IInteractable* Interactable = Cast<IInteractable>(HitResults.GetActor()))
		{
			Interactable->Interact(GetOwner());
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("No Interactable at %s"), *HitResults.GetActor()->GetActorNameOrLabel()));
		}
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No Sweep"));
	}
}

