#include "WeaponInteractable.h"

#include "Common/WeaponHandlerComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/SwordslikeCharacter.h"
#include "Weapons/Weapon.h"

AWeaponInteractable::AWeaponInteractable()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWeaponInteractable::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AWeaponInteractable, WeaponInstance);
}

void AWeaponInteractable::BeginPlay()
{
	Super::BeginPlay();
	
	Collider = GetComponentByClass<USphereComponent>();

	if(HasAuthority())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		WeaponInstance = GetWorld()->SpawnActor<AWeapon>(Weapon, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);

		if(WeaponInstance)
		{
			WeaponInstance->SetReplicates(true);
			// WeaponInstance->SetInteractable(this);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Weapon wasn't spawned successfully"));
		}
	}
}

void AWeaponInteractable::Interact(AActor* InteractingActor)
{
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Interactable: Interact()"));
	
	if (!InteractingActor)
	{
		return;
	}
	
	if (!HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Interact called on Client - Triggering Server RPC"));
		Server_Interact(InteractingActor);
		return;
	}
	
	InteractionProcess(InteractingActor);
}

void AWeaponInteractable::Server_Interact_Implementation(AActor* InteractingActor)
{
	// Multicast_Interact(InteractingActor);
	InteractionProcess(InteractingActor);
}

bool AWeaponInteractable::Server_Interact_Validate(AActor* InteractingActor)
{
	return true;
}

void AWeaponInteractable::InteractionProcess(AActor* InteractingActor)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("INteractable: InteractionProcess()"));
	
	ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(InteractingActor);

	if(!Character)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No Character on interactable"));
		return;
	}

	if(!WeaponInstance)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No Weapon on interactable"));
		return;
	}

	WeaponInstance->SetOwner(InteractingActor);
	Character->GetWeaponHandler()->EquipWeapon(WeaponInstance);

	// disable the interactable
	if(Collider)
	{
		Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	Multicast_Interact(InteractingActor);
}

// for cosmetics 
void AWeaponInteractable::Multicast_Interact_Implementation(AActor* InteractingActor)
{
	if(Collider)
	{
		Collider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AWeaponInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FString AWeaponInteractable::GetInteractionMessage()
{
	if(!WeaponInstance)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			TEXT("No weapon instance on the interactable."));
	}
	
	return FString::Printf(TEXT("Pick up %s"), *WeaponInstance->WeaponName);
}