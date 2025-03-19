 #include "Weapon.h"

#include "Common/WeaponHandlerComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "Player/SwordslikeCharacter.h"

 AWeapon::AWeapon()
{
 	PrimaryActorTick.bCanEverTick = true;

 	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
 	RootComponent = SceneRoot;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>("Mesh");
	Mesh->SetupAttachment(SceneRoot);
	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
 	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
 	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 	
	StartArrow = CreateDefaultSubobject<UArrowComponent>("Start Arrow");
 	StartArrow->SetupAttachment(Mesh);

 	EndArrow = CreateDefaultSubobject<UArrowComponent>("End Arrow");
 	EndArrow->SetupAttachment(Mesh);

 	AreaSphere = CreateDefaultSubobject<USphereComponent>("AreaSphere");
 	AreaSphere->SetupAttachment(SceneRoot);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
 	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

 	if(HasAuthority())
 	{
 		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
 	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

 void AWeapon::OnWeaponEquipped()
 {
 	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 }

 void AWeapon::Interact(AActor* InteractingActor)
 {
 	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Interactable: Interact()"));
	
 	if (!InteractingActor)
 	{
 		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("ERROR: No interacting actor"));
 		return;
 	}
	
 	if (!HasAuthority())
 	{
 		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Interact called on Client - Triggering Server RPC"));
 		Server_Interact(InteractingActor);
 		return;
 	}
	
 	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Interact running on Server"));
 	InteractionProcess(InteractingActor);
 }

void AWeapon::Server_Interact_Implementation(AActor* InteractingActor)
 {
 	InteractionProcess(InteractingActor);
 }

 void AWeapon::Multicast_Interact_Implementation(AActor* InteractingActor)
 {
 	if(AreaSphere)
 	{
 		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 	}
 }

 void AWeapon::InteractionProcess(AActor* InteractingActor)
 {
 	ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(InteractingActor);

 	if(!Character)
 	{
 		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No Character on interactable"));
 		return;
 	}

 	SetOwner(InteractingActor);
 	Character->GetWeaponHandler()->EquipWeapon(this);

 	// disable the interactable
 	if(AreaSphere)
 	{
 		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 	}

 	Multicast_Interact(InteractingActor);
 }

 FString AWeapon::GetInteractionMessage()
 {
 	return FString::Printf(TEXT("Pick up %s"), *WeaponName);
 }

