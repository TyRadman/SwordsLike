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
 	AreaSphere->SetupAttachment(SceneRoot);
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
 	}

 	if(TrailPSC)
 	{
 		TrailPSC->BeginTrails(FName("Trail_Start"), FName("Trail_End"), ETrailWidthMode_FromCentre, 1.f);
 		TrailPSC->SetActive(false);
 	}
 }

void AWeapon::Tick(float DeltaTime)
 {
 	Super::Tick(DeltaTime);

 	// if(!bIsEquipped)
 	// {
 	// 	return;
 	// }
 	//
 	// TArray<FName> Names = Mesh->GetAllSocketNames();
 	// FString Name = FString::Printf(TEXT("Sockets of %s: %d"), *Mesh->GetReadableName(), Names.Num());
	 //
 	// for(FName SocketName : Names)
 	// {
 	// 	Name += FString::Printf(TEXT("%s, "), *SocketName.ToString());
 	// }
 	//
 	// if (!TrailPSC)
 	// {
 	// 	GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Red, TEXT("UpdateTrailEffect: TrailPSC is NULL!"));
 	// 	return;
 	// }
	 //
 	// if (!Mesh)
 	// {
 	// 	GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Red, TEXT("UpdateTrailEffect: Weapon mesh is NULL!"));
 	// 	return;
 	// }
	 //
 	// if (!Mesh->DoesSocketExist(FName("Trail_Start")))
 	// {
 	// 	Names = Mesh->GetAllSocketNames();
 	// 	Name = FString::Printf(TEXT("Sockets of %s: %d"), *Mesh->GetReadableName(), Names.Num());
	 //
		// for(FName SocketName : Names)
		// {
		// 	Name += FString::Printf(TEXT("%s, "), *SocketName.ToString());
		// }
 	// 	
 	// 	GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Yellow, FString::Printf(TEXT("UpdateTrailEffect: Weapon is missing 'Trail_Start' socket!. %s"), *Name));
 	// 	return;
 	// }
	 //
 	// if (!Mesh->DoesSocketExist(FName("Trail_End")))
 	// {
 	// 	Names = Mesh->GetAllSocketNames();
 	// 	Name;
	 //
 	// 	for(FName SocketName : Names)
 	// 	{
 	// 		Name += FString::Printf(TEXT("%s, "), *SocketName.ToString());
 	// 	}
 	// 	
 	// 	GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Yellow, FString::Printf(TEXT("UpdateTrailEffect: Weapon is missing 'Trail_End' socket!. Sockets: %s"), *Name));
 	// 	return;
 	// }
	 //
 	// // If all checks pass, start the trail effect
 	// TrailPSC->BeginTrails(FName("Trail_Start"), FName("Trail_End"), ETrailWidthMode_FromCentre, 1.0f);
	 //
 	// GEngine->AddOnScreenDebugMessage(2, 5.0f, FColor::Green, TEXT("UpdateTrailEffect: Trail effect started successfully!"));
 }

 void AWeapon::OnWeaponEquipped()
 {
 	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
 		// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Interact called on Client - Triggering Server RPC"));
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

 	bIsEquipped = true;
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

