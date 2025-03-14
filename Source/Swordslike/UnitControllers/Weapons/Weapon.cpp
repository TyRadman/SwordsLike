 #include "Weapon.h"

#include "Components/ArrowComponent.h"

 AWeapon::AWeapon()
{
 	PrimaryActorTick.bCanEverTick = true;

 	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
 	RootComponent = SceneRoot;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetupAttachment(SceneRoot);
 	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	StartArrow = CreateDefaultSubobject<UArrowComponent>("Start Arrow");
 	StartArrow->SetupAttachment(Mesh);

 	EndArrow = CreateDefaultSubobject<UArrowComponent>("End Arrow");
 	EndArrow->SetupAttachment(Mesh);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

 void AWeapon::SetInteractable(AWeaponInteractable* Interactable)
 {
	if(!Interactable)
	{
		UE_LOG(LogTemp, Error, TEXT("WEAPON: Interactable is null"));
		return;
	}
 	
 	OwnerInteractable = Interactable;
 }

