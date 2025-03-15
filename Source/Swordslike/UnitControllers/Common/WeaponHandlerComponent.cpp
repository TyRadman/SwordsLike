
#include "WeaponHandlerComponent.h"

#include "DamageInfo.h"
#include "Damagable.h"
#include "KismetTraceUtils.h"
#include "Components/ArrowComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/SwordslikeCharacter.h"
#include "Weapons/Weapon.h"

UWeaponHandlerComponent::UWeaponHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}

void UWeaponHandlerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponHandlerComponent, CurrentWeapon);
	DOREPLIFETIME(UWeaponHandlerComponent, bIsCarryingHeavyWeapon);
}

void UWeaponHandlerComponent::InitEntityComponent(ACharacter* Character)
{
	if(!Character)
	{
		PrintOnScreen_Local(TEXT("UWeaponHandlerComponent: No Character passed"));
		return;
	}

	ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Character);

	AnimInstance = CustomCharacter->GetAnimInstance();
}

void UWeaponHandlerComponent::Setup(ASwordslikeCharacter* Character)
{
	// PrintOnScreen(TEXT("SetUp"));
	
	if (!Character)
	{
		PrintOnScreen(TEXT("ERROR: No Character passed to the weapon handler"), FColor::Red, 10.f);
		return;
	}

	if(!StartingWeapon)
	{
		PrintOnScreen(TEXT("Weapon BP is missing."));
		return;
	}

	WeaponOwner = Character;

	Server_SpawnDefaultWeapon(StartingWeapon);
}

void UWeaponHandlerComponent::Server_SpawnDefaultWeapon_Implementation(TSubclassOf<AWeapon> WeaponClass)
{
	if (!WeaponOwner || !WeaponClass)
	{
		PrintOnScreen(TEXT("Invalid spawn parameters."), FColor::Red);
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = WeaponOwner;
	SpawnParams.Instigator = WeaponOwner->GetInstigator();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AWeapon* SpawnedWeapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass, SpawnParams);

	if (SpawnedWeapon)
	{
		PrintOnScreen(TEXT("Weapon spawned successfully on Server."), FColor::Green);
		CurrentWeapon = SpawnedWeapon;
		EquipWeapon(SpawnedWeapon);
	}
	else
	{
		PrintOnScreen(TEXT("Weapon spawn failed on Server!"), FColor::Red);
	}
}

bool UWeaponHandlerComponent::Server_SpawnDefaultWeapon_Validate(TSubclassOf<AWeapon> WeaponClass)
{
	return true;
}

void UWeaponHandlerComponent::OnRep_CurrentWeapon()
{
	if(CurrentWeapon)
	{
		EquipWeapon(CurrentWeapon);
	}
}

void UWeaponHandlerComponent::EquipWeapon(AWeapon* Weapon)
{
	if(HasAuthority())
	{
		EquipWeaponProcess(Weapon);
	}
	else
	{
		Server_EquipWeapon(Weapon);
	}
}

void UWeaponHandlerComponent::Server_EquipWeapon_Implementation(AWeapon* Weapon)
{
	EquipWeaponProcess(Weapon);
}

bool UWeaponHandlerComponent::Server_EquipWeapon_Validate(AWeapon* Weapon)
{
	return true;
}

void UWeaponHandlerComponent::EquipWeaponProcess(AWeapon* Weapon)
{
	if(!Weapon)
	{
		PrintOnScreen(FString::Printf(TEXT("WeaponHandler: No Weapon passed for EquipWeapon")));
		return;
	}
	
	if (!WeaponOwner || !WeaponOwner->GetCustomMesh())
	{
		PrintOnScreen(TEXT("WeaponHandler: WeaponOwner or CustomMesh is null"), FColor::Red);
		return;
	}

	bIsCarryingHeavyWeapon = Weapon->bIsCarryingHeavyWeapon;
	
	CurrentWeapon = Weapon;
	CurrentWeapon->AttachToComponent(
		WeaponOwner->GetCustomMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("swordsocket_r")
	);

	CurrentWeapon->SetActorRelativeLocation(CurrentWeapon->LocationOffset);
	CurrentWeapon->SetActorRelativeRotation(CurrentWeapon->RotationOffset);

	PlayEquipMontage();
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
	if (!CurrentWeapon)
	{
		PrintOnScreen(TEXT("CurrentWeapon is null"), FColor::Red);
		return;
	}

	if (!CurrentWeapon->StartArrow || !CurrentWeapon->EndArrow)
	{
		PrintOnScreen(TEXT("Weapon arrows are null"), FColor::Red);
		return;
	}
	
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
		PrintOnScreen(TEXT("No arrows"));
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
				DamageInfo.PostureDamage = CurrentWeapon->PostureDamagePerHit;
				DamageInfo.Instigator = WeaponOwner;
				
				TargetDamagable->TakeDamage(DamageInfo);
				TargetsHit.Add(TargetDamagable);
				break;
			}
		}
	}
}

#pragma region Animations
void UWeaponHandlerComponent::PlayEquipMontage()
{
	PrintOnScreen(FString::Printf(TEXT("WeaponHandler: Playing animation 0")));
	
	if(!AnimInstance)
	{
		PrintOnScreen(FString::Printf(TEXT("WeaponHandler: No Anim Instance")));
	}
	
	PlayMontage(EquipMontage);
}

void UWeaponHandlerComponent::PlayMontage(UAnimMontage* Montage)
{
	PrintOnScreen(FString::Printf(TEXT("WeaponHandler: Playing animation 1")));
	AnimInstance->Montage_Play(Montage);

	if(!HasAuthority())
	{
		Server_PlayMontage(Montage);
	}
	else
	{
		Multicast_PlayMontage(Montage);
	}
}

void UWeaponHandlerComponent::Server_PlayMontage_Implementation(UAnimMontage* Montage)
{
	Multicast_PlayMontage(Montage);
}

void UWeaponHandlerComponent::Multicast_PlayMontage_Implementation(UAnimMontage* Montage)
{
	// if(GetOwnerRole() == ROLE_AutonomousProxy)
	// {
	// 	return;
	// }

	AnimInstance->Montage_Play(Montage);
}
#pragma endregion
