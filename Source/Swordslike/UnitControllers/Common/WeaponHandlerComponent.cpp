
#include "WeaponHandlerComponent.h"

#include "DamageInfo.h"
#include "Damagable.h"
#include "KismetTraceUtils.h"
#include "Components/ArrowComponent.h"
#include "Player/SwordslikeCharacter.h"
#include "Weapons/Weapon.h"

UWeaponHandlerComponent::UWeaponHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWeaponHandlerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWeaponHandlerComponent::Setup(ASwordslikeCharacter* Character)
{
	if (!Character)
	{
		PrintOnScreen_Local(FString::Printf(TEXT("ERROR: No Character passed to the weapon handler")), FColor::Red, 10.f);
		return;
	}

	WeaponOwner = Character;

	if(!StartingWeapon)
	{
		PrintOnScreen_Local(TEXT("Weapon BP is missing."));
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Instigator = WeaponOwner;
	SpawnParams.Owner = WeaponOwner;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	
	AWeapon* Weapon = GetWorld()->SpawnActor<AWeapon>(StartingWeapon, SpawnParams);

	if(Weapon)
	{
		EquipWeapon(Weapon);
	}
	else
	{
		PrintOnScreen_Local(TEXT("No weapon created, are you missing a subclass reference?"));
	}
}

void UWeaponHandlerComponent::EquipWeapon(AWeapon* Weapon)
{
	if(Weapon)
	{
		CurrentWeapon = Weapon;

		CurrentWeapon->AttachToComponent(WeaponOwner->GetCustomMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, FName("swordsocket_r"));

		CurrentWeapon->SetActorRelativeLocation(CurrentWeapon->LocationOffset);
		CurrentWeapon->SetActorRelativeRotation(CurrentWeapon->RotationOffset);
	}
	else
	{
		PrintOnScreen_Local(FString::Printf(TEXT("WeaponHandler: No Weapon passed for EquipWeapon")));
	}
}

float UWeaponHandlerComponent::GetWeaponStaminaCost() const
{
	return CurrentWeapon->StaminaPerHit;
}

FVector UWeaponHandlerComponent::GetWeaponMiddleLocation() const
{
	return (CurrentWeapon->StartArrow->GetComponentLocation() + CurrentWeapon->EndArrow->GetComponentLocation()) / 2.f;
}

void UWeaponHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(bIsAttacking)
	{
		GetTargetsInWeaponRange();
	}
}

void UWeaponHandlerComponent::StartWeaponAttackDetection()
{
	if(OnWeaponHitStarted.IsBound())
	{
		OnWeaponHitStarted.Broadcast(CurrentWeapon);
	}
	
	PreviousStartLocation = CurrentWeapon->StartArrow->GetComponentLocation();
	PreviousEndLocation = CurrentWeapon->EndArrow->GetComponentLocation();
	bIsAttacking = true;
}

void UWeaponHandlerComponent::StopWeaponAttackDetection()
{
	bIsAttacking = false;
	TargetsHit.Empty();
}

void UWeaponHandlerComponent::GetTargetsInWeaponRange()
{
	if(!CurrentWeapon->StartArrow || !CurrentWeapon->EndArrow)
	{
		PrintOnScreen_Local(TEXT("No arrows"));
		return;
	}

	FVector StartLocation = CurrentWeapon->StartArrow->GetComponentLocation();
	FVector EndLocation = CurrentWeapon->EndArrow->GetComponentLocation();
	
	CacheTargetsBetweenTwoPoints(PreviousStartLocation, StartLocation);
	CacheTargetsBetweenTwoPoints(PreviousEndLocation, EndLocation);
	CacheTargetsBetweenTwoPoints(PreviousStartLocation, EndLocation);
	CacheTargetsBetweenTwoPoints(PreviousEndLocation, StartLocation);

	CacheTargetsBetweenTwoPoints(StartLocation, EndLocation);

	PreviousStartLocation = CurrentWeapon->StartArrow->GetComponentLocation();
	PreviousEndLocation = CurrentWeapon->EndArrow->GetComponentLocation();
}

void UWeaponHandlerComponent::CacheTargetsBetweenTwoPoints(const FVector& StartLocation, const FVector& EndLocation)
{
	float Radius = CurrentWeapon->HitBoxRadius;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	TArray<FHitResult> HitResults;
	
	bool bHitStart = GetWorld()->SweepMultiByObjectType(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Radius),
		QueryParams);

	DrawDebugSweptSphere(GetWorld(), StartLocation, EndLocation, Radius, FColor::Red, false, .5f);
	
	GetTargetsFromHitResults(HitResults);
}

void UWeaponHandlerComponent::GetTargetsFromHitResults(TArray<FHitResult>& HitResults)
{
	for(const FHitResult Result : HitResults)
	{
	if(!Result.GetActor())
	{
		continue;
	}

	AActor* Actor = Result.GetActor();
	UE_LOG(LogTemp, Log, TEXT("%s"), *Result.GetActor()->GetActorNameOrLabel());

	TArray<UActorComponent*> Components;
	Actor->GetComponents(Components);

	for(UActorComponent* Component : Components)
	{
		if(!Component)
		{
			continue;
		}
		
		if(IDamagable* TargetDamagable = Cast<IDamagable>(Component))
		{
			// if the target has already been hit in this attack, then skip it
			if(TargetsHit.Contains(TargetDamagable))
			{
				break;
			}
				
			FDamageInfo DamageInfo;
			DamageInfo.Damage = CurrentWeapon->DamagePerHit;
			DamageInfo.Instigator = WeaponOwner;
				
			TargetDamagable->TakeDamage(DamageInfo);
			TargetsHit.Add(TargetDamagable);
			break;
		}
	}
}
}

