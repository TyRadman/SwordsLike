#include "WeaponAnimNotifyState_Trail.h"

#include "Common/WeaponHandlerComponent.h"
#include "Components/ArrowComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/SwordslikeCharacter.h"
#include "Weapons/Weapon.h"

void UWeaponAnimNotifyState_Trail::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);
    // Early out if running on a dedicated server
    if (MeshComp->GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    // Get the character and weapon
    ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner());
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon Trail Notify: No character found for trail notify in anim: %s"), *GetPathNameSafe(Animation));
        return;
    }

    AWeapon* Weapon = Character->GetWeaponHandler()->GetCurrentWeapon(); // Assuming you have a method to get the current weapon
    if (!Weapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon Trail Notify: No weapon found for trail notify in anim: %s"), *GetPathNameSafe(Animation));
        return;
    }

    Weapon->GetTrailEffect()->SetActive(true);
}

void UWeaponAnimNotifyState_Trail::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::NotifyEnd(MeshComp, Animation);

    // Early out if running on a dedicated server
    if (MeshComp->GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

    // Get the character and weapon
    ASwordslikeCharacter* Character = Cast<ASwordslikeCharacter>(MeshComp->GetOwner());
    if (!Character)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon Trail Notify: No character found for trail notify in anim: %s"), *GetPathNameSafe(Animation));
        return;
    }

    AWeapon* Weapon = Character->GetWeaponHandler()->GetCurrentWeapon(); // Assuming you have a method to get the current weapon
    if (!Weapon)
    {
        UE_LOG(LogTemp, Warning, TEXT("Weapon Trail Notify: No weapon found for trail notify in anim: %s"), *GetPathNameSafe(Animation));
        return;
    }

    Weapon->GetTrailEffect()->SetActive(false);
}

