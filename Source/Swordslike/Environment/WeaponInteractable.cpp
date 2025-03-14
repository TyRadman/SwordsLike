// Fill out your copyright notice in the Description page of Project Settings.


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

void AWeaponInteractable::Interact(AActor* InteractingActor)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Interactable: Interact()"));
	
	if (!InteractingActor)
	{
		return;
	}
	
	if (HasAuthority())
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Interact running on Server"));
		Multicast_Interact(InteractingActor);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Interact called on Client - Triggering Server RPC"));
		Server_Interact(InteractingActor);
	}
}

void AWeaponInteractable::Server_Interact_Implementation(AActor* InteractingActor)
{
	Multicast_Interact(InteractingActor);
}


void AWeaponInteractable::Multicast_Interact_Implementation(AActor* InteractingActor)
{
	InteractionProcess(InteractingActor);
}

void AWeaponInteractable::InteractionProcess(AActor* InteractingActor)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Interacting Actor"));
	ASwordslikeCharacter* Character;

	if(ASwordslikeCharacter* CastCharacter = Cast<ASwordslikeCharacter>(InteractingActor))
	{
		Character = CastCharacter;
	}
	else
	{
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
}

void AWeaponInteractable::BeginPlay()
{
	Super::BeginPlay();

	if(HasAuthority())
	{
		Collider = GetComponentByClass<USphereComponent>();
	
		FActorSpawnParameters SpawnParams;
		SpawnParams.Instigator = GetInstigator();
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		WeaponInstance = GetWorld()->SpawnActor<AWeapon>(Weapon, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
		WeaponInstance->SetReplicates(true);
		WeaponInstance->SetInteractable(this);
	}
}

void AWeaponInteractable::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

