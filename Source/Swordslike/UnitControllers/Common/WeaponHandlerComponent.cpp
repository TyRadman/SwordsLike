
#include "WeaponHandlerComponent.h"
#include "DamageInfo.h"
#include "Damagable.h"
#include "KismetTraceUtils.h"
#include "Components/ArrowComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/SwordslikeCharacter.h"
#include "Weapons/Weapon.h"

UWeaponHandlerComponent::UWeaponHandlerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);
}

void UWeaponHandlerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponHandlerComponent, CurrentWeapon);
	DOREPLIFETIME_CONDITION(UWeaponHandlerComponent, bIsCarryingHeavyWeapon, COND_OwnerOnly);
}

void UWeaponHandlerComponent::InitEntityComponent(ACharacter* Character)
{
	if(!Character)
	{
		PrintOnScreen_Local(TEXT("UWeaponHandlerComponent: No Character passed"));
		return;
	}

	if(ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Character))
	{
		PlayerCharacter = CustomCharacter;
		bIsLocallyController = PlayerCharacter->IsLocallyControlled();
		AnimInstance = CustomCharacter->GetAnimInstance();
		OnWeaponHitStarted.AddUObject(CustomCharacter->GetSprintComponent(), &USprintComponent::OnWeaponHit);
		WeaponOwner = CustomCharacter;
	}
}

void UWeaponHandlerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if(!bIsLocallyController)
	{
		return;
	}
	
	if(bIsAttacking)
	{
		GetTargetsInWeaponRange();
	}
}

UAnimMontage* UWeaponHandlerComponent::GetAttackMontage() const
{
	if(!CurrentWeapon)
	{
		PrintOnScreen(TEXT("No attack montage"));
		return nullptr;
	}

	return CurrentWeapon->ComboMontage;
}

UAnimMontage* UWeaponHandlerComponent::GetNextAttackMontage()
{
	if(!CurrentWeapon)
	{
		PrintOnScreen_Local(TEXT("UWeaponHandlerComponent: ERROR, no weapon"));
		return nullptr;
	}

	if(CurrentAttackAnimationIndex >= CurrentWeapon->ComboMontages.Num())
	{
		return nullptr;
	}
	
	UAnimMontage* NextMontage = CurrentWeapon->ComboMontages[CurrentAttackAnimationIndex];
	CurrentAttackAnimationIndex++;
	return NextMontage;
}

void UWeaponHandlerComponent::ResetAttackMontages()
{
	CurrentAttackAnimationIndex = 0;
}

const UAnimMontage* UWeaponHandlerComponent::GetCurrentAttackMontage()
{
	return CurrentWeapon->ComboMontages[CurrentAttackAnimationIndex];
}

void UWeaponHandlerComponent::OnRep_CurrentWeapon()
{
	// PrintOnScreen(FString::Printf(TEXT("Weapon Set 2")));
}

void UWeaponHandlerComponent::EquipWeapon(AWeapon* Weapon)
{
	if(!HasAuthority())
	{
		Server_EquipWeapon(Weapon);
	}
	else
	{
		EquipWeaponProcess(Weapon);
	}
}

void UWeaponHandlerComponent::Server_EquipWeapon_Implementation(AWeapon* Weapon)
{
	EquipWeaponProcess(Weapon);
}

void UWeaponHandlerComponent::EquipWeaponProcess(AWeapon* Weapon)
{
	if(!Weapon)
	{
		PrintOnScreen(FString::Printf(TEXT("WeaponHandler ERROR: No Weapon passed for EquipWeapon")));
		return;
	}
	
	if (!WeaponOwner || !WeaponOwner->GetCustomMesh())
	{
		PrintOnScreen(TEXT("ERROR: WeaponHandler: WeaponOwner or CustomMesh is null"), FColor::Red);
		return;
	}

	bIsCarryingHeavyWeapon = Weapon->bIsCarryingHeavyWeapon;

	// PrintOnScreen(FString::Printf(TEXT("Equipped weapon successfully (%s)"), *Weapon->GetActorNameOrLabel()), FColor::Green);
	CurrentWeapon = Weapon;
	
	Weapon->AttachToComponent(
		WeaponOwner->GetCustomMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		FName("swordsocket_r")
	);

	CurrentWeapon->SetActorRelativeLocation(CurrentWeapon->LocationOffset);
	CurrentWeapon->SetActorRelativeRotation(CurrentWeapon->RotationOffset);
	CurrentWeapon->OnWeaponEquipped();

	if(UWidgetComponent* WeaponIndicator = WeaponOwner->GetAttackIndicatorWidgetComponent())
	{
		if(CurrentWeapon->GetMesh())
		{
			WeaponIndicator->AttachToComponent(
				CurrentWeapon->GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				FName("Indicator"));
		}
		else
		{
			PrintOnScreen(TEXT("UWeaponHandlerComponent ERROR: no mesh on weapon"));
		}
	}
	else
	{
		PrintOnScreen(TEXT("UWeaponHandlerComponent ERROR: no Widget on player"));
	}

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

void UWeaponHandlerComponent::GetTargetsInWeaponRange()
{
	if(!CurrentWeapon->StartArrow || !CurrentWeapon->EndArrow)
	{
		PrintOnScreen(TEXT("No arrows"));
		return;
	}

	const FVector StartLocation = CurrentWeapon->StartArrow->GetComponentLocation();
	const FVector EndLocation = CurrentWeapon->EndArrow->GetComponentLocation();
	
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
	const float Radius = CurrentWeapon->HitBoxRadius;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	TArray<FHitResult> HitResults;
	
	const bool bHitStart = GetWorld()->SweepMultiByObjectType(
		HitResults,
		StartLocation,
		EndLocation,
		FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn),
		FCollisionShape::MakeSphere(Radius),
		QueryParams);

	// DrawDebugSweptSphere(GetWorld(), StartLocation, EndLocation, Radius, FColor::Red, false, .5f);

	if(bHitStart)
	{
		GetTargetsFromHitResults(HitResults);
	}
}

void UWeaponHandlerComponent::GetTargetsFromHitResults(TArray<FHitResult>& HitResults)
{
	for(const FHitResult Result : HitResults)
	{
		AActor* TargetActor = Result.GetActor();
		if(!TargetActor)
		{
			continue;
		}
		
		TArray<UActorComponent*> Components;
		TargetActor->GetComponents(Components);

		for(UActorComponent* Component : Components)
		{
			if(!Component)
			{
				continue;
			}
		
			if(IDamagable* TargetDamagable = Cast<IDamagable>(Component))
			{
				if(TargetDamagable->IsInvincible())
				{
					break;
				}
				
				// if the target has already been hit in this attack, then skip it
				if(TargetsHit.Contains(TargetDamagable))
				{
					break;
				}
				
				FDamageInfo DamageInfo;
				// DamageInfo.Damage = CurrentWeapon->DamagePerHit;
				DamageInfo.Damage = CurrentDamage;
				DamageInfo.PostureDamage = CurrentWeapon->PostureDamagePerHit;
				DamageInfo.DamageInstigator = WeaponOwner;
				DamageInfo.DamageInstigatorCharacter = WeaponOwner;
				DamageInfo.ImpactLocation = Result.ImpactPoint;
				DamageInfo.HitType = CurrentHitType;
				
				if(HasAuthority())
				{
					TargetDamagable->TakeDamage(DamageInfo);
				}
				else
				{
					Server_InflictDamage(TargetActor, DamageInfo);
				}
				
				WeaponOwner->PerformCameraShake(CurrentCameraShake);

				if(TargetDamagable->IsAlive())
				{
					if(!HasAuthority())
					{
						Server_OnTargetAttacked(Result.ImpactPoint, CurrentWeapon);
					}
					else
					{
						Multicasat_OnTargetAttacked(Result.ImpactPoint, CurrentWeapon);
					}
				}
				
				TargetsHit.Add(TargetDamagable);
				break;
			}
		}
	}
}

void UWeaponHandlerComponent::Server_InflictDamage_Implementation(AActor* Target, const FDamageInfo& DamageInfo)
{
	if (!Target)
	{
		return;
	}
	
	TArray<UActorComponent*> Components;
	Target->GetComponents(Components);

	for (UActorComponent* Component : Components)
	{
		if (IDamagable* Damagable = Cast<IDamagable>(Component))
		{
			Damagable->TakeDamage(DamageInfo);
			break;
		}
	}
}

void UWeaponHandlerComponent::StartWeaponAttackDetection(const EHitType NewHitType, const float NewDamage, const TSubclassOf<UCameraShakeBase>& CameraShake)
{
	if (!CurrentWeapon)
	{
		PrintOnScreen(FString::Printf(TEXT("CurrentWeapon is null on %s"), *UEnum::GetValueAsString(GetOwnerRole())), FColor::Red);
		return;
	}

	if (!CurrentWeapon->StartArrow || !CurrentWeapon->EndArrow)
	{
		PrintOnScreen(TEXT("Weapon arrows are null"), FColor::Red);
		return;
	}
	
	CurrentHitType = NewHitType;
	CurrentDamage = NewDamage;

	if(!CameraShake)
	{
		CurrentCameraShake = WeaponOwner->CameraShake;
	}
	else
	{
		CurrentCameraShake = CameraShake;
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

void UWeaponHandlerComponent::Server_OnTargetAttacked_Implementation(const FVector ImpactPoint, AWeapon* Weapon)
{
	Multicasat_OnTargetAttacked(ImpactPoint, Weapon);
}

void UWeaponHandlerComponent::Multicasat_OnTargetAttacked_Implementation(const FVector ImpactPoint, AWeapon* Weapon)
{
	if(Weapon)
	{
		if(Weapon->ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Weapon->ImpactParticles, ImpactPoint);
		}
		else
		{
			PrintOnScreen(TEXT("Weapon impact particles is null"));
		}
	}
	else
	{
		PrintOnScreen(TEXT("Weapon is null"));
	}
}

#pragma region Animations
void UWeaponHandlerComponent::PlayEquipMontage()
{
	if(!AnimInstance)
	{
		PrintOnScreen(FString::Printf(TEXT("WeaponHandler: No Anim Instance")));
		return;
	}
	
	PlayMontage(EquipMontage);
}

void UWeaponHandlerComponent::PlayMontage(UAnimMontage* Montage)
{
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

void UWeaponHandlerComponent::OnForceStopAttack()
{
	bIsAttacking = false;
	TargetsHit.Empty();
}

void UWeaponHandlerComponent::Server_PlayMontage_Implementation(UAnimMontage* Montage)
{
	Multicast_PlayMontage(Montage);
}

void UWeaponHandlerComponent::Multicast_PlayMontage_Implementation(UAnimMontage* Montage)
{
	if(AnimInstance)
	{
		AnimInstance->Montage_Play(Montage);
	}
}
#pragma endregion
