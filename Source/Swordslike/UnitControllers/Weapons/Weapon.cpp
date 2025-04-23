#include "Weapon.h"
#include "NiagaraComponent.h"
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
 	AreaSphere->SetupAttachment(Mesh);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
 	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

 	TrailEffect = CreateDefaultSubobject<UNiagaraComponent>("Weapon Trail Effect");
 	TrailEffect->SetupAttachment(Mesh);

 	TrailPSC = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TrailPSC"));
 	TrailPSC->SetupAttachment(Mesh);

}

void AWeapon::BeginPlay()
 {
	 Super::BeginPlay();

 	if(HasAuthority())
 	{
 		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 		AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
 	
 		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 		Mesh->SetCollisionResponseToAllChannels(ECR_Block);
 		Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
 	}

 	if(TrailPSC)
 	{
 		TrailPSC->BeginTrails(FName("Trail_Start"), FName("Trail_End"), ETrailWidthMode_FromCentre, 1.f);
 		TrailPSC->SetActive(false);
 	}
 	else
 	{
 		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, TEXT("No trail effect"));
 	}

 	RelativeLocation = Mesh->GetRelativeLocation();
 	RelativeRotation = Mesh->GetRelativeRotation();
 }

 void AWeapon::OnWeaponEquipped()
 {
 }

 void AWeapon::Interact(AActor* InteractingActor)
 {
	 if (!InteractingActor)
	 {
	 	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("ERROR: No interacting actor"));
	 	return;
	 }

 	SetOwner(InteractingActor);
	
 	if (!HasAuthority())
 	{
 		Server_Interact(InteractingActor);
 	}
 	else
 	{
 		InteractionProcess(InteractingActor);
 	}
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
 	const ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(InteractingActor);
 	
 	if(!Character)
 	{
 		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("No Character on interactable"));
 		return;
 	}

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

void AWeapon::OnEquipped()
{
 	bIsEquipped = true;
 	
 	Mesh->SetSimulatePhysics(false);
 	
 	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 	
 	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

 	Mesh->SetRelativeLocation(RelativeLocation);
 	Mesh->SetRelativeRotation(RelativeRotation);
}

void AWeapon::OnDropped()
 {
 	bIsEquipped = false;
 	
 	Mesh->SetSimulatePhysics(true);

 	FTimerHandle TimerHandle;
 	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 	Mesh->SetCollisionResponseToAllChannels(ECR_Block);
 	Mesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
 	Mesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
 	
 	GetWorld()->GetTimerManager().SetTimer(
 		TimerHandle,
 		[this]()
 		{
 			Mesh->SetSimulatePhysics(false);
 			
 			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
 			AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
 		},
 		1.0f,
 		false);
}

