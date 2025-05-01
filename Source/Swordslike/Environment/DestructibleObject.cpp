#include "DestructibleObject.h"

#include "Common/DamageInfo.h"
#include "Components/ArrowComponent.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include "GeometryCollection/GeometryCollectionComponent.h"

ADestructibleObject::ADestructibleObject()
{
	PrimaryActorTick.bCanEverTick = false;
	
	DestructibleMesh = CreateDefaultSubobject<UStaticMeshComponent>("Destructible Mesh");
	DestructibleMesh->SetupAttachment(RootComponent);
	
	GeometryCollectionComponent = CreateDefaultSubobject<UGeometryCollectionComponent>("Geometry Collection Component");
	GeometryCollectionComponent->SetupAttachment(DestructibleMesh);
	GeometryCollectionComponent->SetSimulatePhysics(false);
	GeometryCollectionComponent->SetVisibility(false);
	GeometryCollectionComponent->SetVisibility(false);
	GeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	ImpulsePoint = CreateDefaultSubobject<UArrowComponent>("Impulse Point");
}


void ADestructibleObject::BeginPlay()
{
	Super::BeginPlay();

	
}

void ADestructibleObject::TakeDamage(const FDamageInfo& DamageInfo)
{
	if (!GeometryCollectionComponent || !DestructibleMesh)
	{
		return;
	}
	
	ActivateGeometryCollection(DamageInfo);
	
	// if(HasAuthority())
	// {
		// DestructibleMesh->SetVisibility(false);
		// DestructibleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// DestructibleMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		
		Multicast_SetupColliders(DamageInfo);
	// }
	// else
	// {
	// 	Server_SetupColliders(DamageInfo);
	// }
}

void ADestructibleObject::Server_SetupColliders_Implementation(const FDamageInfo& DamageInfo)
{
	PerformSetupColliders(DamageInfo);
}

void ADestructibleObject::Multicast_SetupColliders_Implementation(const FDamageInfo& DamageInfo)
{
	ActivateGeometryCollection(DamageInfo);
}

void ADestructibleObject::PerformSetupColliders(const FDamageInfo& DamageInfo)
{
	Multicast_SetupColliders(DamageInfo);
}

void ADestructibleObject::ActivateGeometryCollection(const FDamageInfo& DamageInfo)
{
	DestructibleMesh->SetVisibility(false);
	DestructibleMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	DestructibleMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	
	// Enable geometry collection
	GeometryCollectionComponent->SetVisibility(true);
	GeometryCollectionComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	GeometryCollectionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	GeometryCollectionComponent->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	GeometryCollectionComponent->SetSimulatePhysics(true);
	GeometryCollectionComponent->WakeAllRigidBodies();
	FVector Origin = GetActorLocation();
	Origin.Y += 10.f;
	GeometryCollectionComponent->AddRadialImpulse(DamageInfo.ImpactLocation, ImpulseRadius, ImpulseForce, RIF_Constant, true);

	
	DynamicMaterials.Empty();
	const int32 NumMats = GeometryCollectionComponent->GetNumMaterials();
	for (int32 i = 0; i < NumMats; ++i)
	{
		if (UMaterialInstanceDynamic* DynMat = GeometryCollectionComponent->CreateAndSetMaterialInstanceDynamic(i))
		{
			DynamicMaterials.Add(DynMat);
		}
	}
	
	GetWorldTimerManager().SetTimer(FadeTimerHandle, this, &ADestructibleObject::FadeOutDebris, 0.02f, true, 3.0f);
}

void ADestructibleObject::FadeOutDebris()
{
	constexpr float FadeSpeed = 0.25f;
	CurrentFade -= FadeSpeed * GetWorld()->GetDeltaSeconds();

	for (UMaterialInstanceDynamic* Mat : DynamicMaterials)
	{
		if (Mat)
		{
			Mat->SetScalarParameterValue("Fade", CurrentFade);
		}
	}

	if (CurrentFade <= 0.f)
	{
		GetWorldTimerManager().ClearTimer(FadeTimerHandle);
		Destroy();
	}
}

