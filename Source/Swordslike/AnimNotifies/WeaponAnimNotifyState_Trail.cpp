#include "WeaponAnimNotifyState_Trail.h"

#include "Common/WeaponHandlerComponent.h"
#include "Components/ArrowComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/SwordslikeCharacter.h"
#include "Weapons/Weapon.h"

void UWeaponAnimNotifyState_Trail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
    
    if (MeshComp->GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
 		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, TEXT("UWeaponAnimNotifyState_Trail: ERROR: Is Dedicated server"));
        return;
    }

    const ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner());
    if (!Character)
    {
 		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, TEXT("Weapon Trail Notify: No character found for trail notify in anim: %s"), *GetPathNameSafe(Animation));
        return;
    }

    const AWeapon* Weapon = Character->GetWeaponHandler()->GetCurrentWeapon(); // Assuming you have a method to get the current weapon
    if (!Weapon)
    {
 		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, TEXT("Weapon Trail Notify: No weapon found for trail notify in anim: %s"), *GetPathNameSafe(Animation));
        return;
    }

    Weapon->GetTrailEffect()->SetActive(true);
}

void UWeaponAnimNotifyState_Trail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::NotifyEnd(MeshComp, Animation);

    if (MeshComp->GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
 		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, TEXT("UWeaponAnimNotifyState_Trail: ERROR: Is Dedicated server"));
        return;
    }

    const ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner());
    if (!Character)
    {
 		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, TEXT("Weapon Trail Notify: No character found for trail notify in anim: %s"), *GetPathNameSafe(Animation));
        return;
    }

    const AWeapon* Weapon = Character->GetWeaponHandler()->GetCurrentWeapon();
    if (!Weapon)
    {
 		// GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Purple, TEXT("Weapon Trail Notify: No weapon found for trail notify in anim: %s"), *GetPathNameSafe(Animation));
        return;
    }

    Weapon->GetTrailEffect()->SetActive(false);
}

