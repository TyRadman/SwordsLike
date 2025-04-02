#include "MyActorComponent.h"

#include <string>

UMyActorComponent::UMyActorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMyActorComponent::BeginPlay()
{
	Super::BeginPlay();
}

#pragma region Debug
void UMyActorComponent::PrintOnScreen_Local(const int32 Key, const FString& Message) const
{
	if(IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(Key, 5.f,  FColor::Red, Message);
	}
}

void UMyActorComponent::PrintOnScreen_Local(const FString& Message) const
{
	if(IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f,  FColor::Red, Message);
	}
}

void UMyActorComponent::PrintOnScreen_Local(const FString& Message, FColor Color) const
{ 
	if(IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, Color, Message);
	}
}

void UMyActorComponent::PrintOnScreen_Local(const FString& Message, FColor Color, float Duration) const
{
	if(IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
	}
}

void UMyActorComponent::PrintOnScreen_Local(const FString& Message, float Duration) const
{
	if(IsLocallyControlled())
	{
		GEngine->AddOnScreenDebugMessage(-1, Duration, FColor::Black, Message);
	}
}

void UMyActorComponent::PrintOnScreen(const FString& Message) const
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f,  FColor::Red, Message);
}

void UMyActorComponent::PrintOnScreen(const FString& Message, FColor Color) const
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, Color, Message);
}

void UMyActorComponent::PrintOnScreen(const FString& Message, FColor Color, float Duration)
{
	GEngine->AddOnScreenDebugMessage(-1, Duration, Color, Message);
}

bool UMyActorComponent::HasAuthority() const
{
	return GetOwner()->HasAuthority();
}

bool UMyActorComponent::IsAutonomousProxy() const
{
	return IsLocallyControlled();
}
#pragma endregion

void UMyActorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

