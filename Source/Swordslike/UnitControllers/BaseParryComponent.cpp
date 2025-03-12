#include "BaseParryComponent.h"

#include "GameFramework/Character.h"
#include "Player/PlayerCombatComponent.h"
#include "Player/SwordslikeCharacter.h"

UBaseParryComponent::UBaseParryComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBaseParryComponent::InitEntityComponent(ACharacter* Character)
{
	if(!Character)
	{
		PrintOnScreen_Local(TEXT("No Character passed to the Parry Component"));
		return;
	}

	OwnerCharacter = Character;
	AnimInstance = OwnerCharacter->GetMesh()->GetAnimInstance();

	AnimInstance->OnPlayMontageNotifyBegin.AddDynamic(this, &UBaseParryComponent::OnParryNotifyStart);
}

void UBaseParryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AddToCurrentPosture(PostureRecoveryRate * DeltaTime);
}

#pragma region Parry Network
void UBaseParryComponent::Parry()
{
	if(OnParryStartedEvent.IsBound())
	{
		OnParryStartedEvent.Broadcast();
	}
	
	if(!HasAuthority())
	{
		Server_Parry();
		return;
	}
	
	AnimInstance->Montage_Play(ParryMontage);
	AnimInstance->Montage_SetNextSection(StartSectionName, MiddleSectionName);
}

void UBaseParryComponent::Server_Parry_Implementation()
{
	Multicast_Parry();
}

void UBaseParryComponent::Multicast_Parry_Implementation()
{
	AnimInstance->Montage_Play(ParryMontage);
	AnimInstance->Montage_SetNextSection(StartSectionName, MiddleSectionName);
}
#pragma endregion

#pragma region End Parry Network
void UBaseParryComponent::EndParry()
{
	if(OnParryEndedEvent.IsBound())
	{
		OnParryEndedEvent.Broadcast();
	}

	if(!HasAuthority())
	{
		Server_EndParry();
	}

	CurrentParryState = EParryState::None;
	AnimInstance->Montage_JumpToSection(EndSectionName, ParryMontage);
	AnimInstance->Montage_Stop(0.3f, ParryMontage);
}

void UBaseParryComponent::Server_EndParry_Implementation()
{
	Multicast_EndParry();
}

void UBaseParryComponent::Multicast_EndParry_Implementation()
{
	AnimInstance->Montage_JumpToSection(EndSectionName, ParryMontage);
	AnimInstance->Montage_Stop(0.3f, ParryMontage);
}
#pragma endregion

EParryState UBaseParryComponent::ValidateParry(const FDamageInfo& DamageInfo) const
{
	AActor* AttackSource = DamageInfo.Instigator;

	ASwordslikeCharacter* AttackerCharacter = Cast<ASwordslikeCharacter>(AttackSource);

	// check if the attack is caused by another character, otherwise, no parry takes place
	if(!AttackerCharacter)
	{
		return EParryState::None;
	}
	
	// check if the damage instigate is within the parry range
	FVector OwnerForward = OwnerCharacter->GetActorForwardVector();
	FVector DirectionToAttacker = (AttackSource->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal();

	float DotProduct = FVector::DotProduct(OwnerForward, DirectionToAttacker);
	
	if(DotProduct < 0.5f)
	{
		return EParryState::None;
	}

	if(CurrentParryState == EParryState::Perfect)
	{
		AttackerCharacter->GetCombat()->ForceStopAttack();
	}
	
	return CurrentParryState;
}

#pragma region Notifies
void UBaseParryComponent::OnParryNotifyStart(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	if(!ParryStates.Contains(NotifyName))
	{
		return;
	}
	
	CurrentParryState = ParryStates[NotifyName];
	// PrintOnScreen(FString::Printf(TEXT("Parry state %s"), *UEnum::GetValueAsString(CurrentParryState)));
}
#pragma endregion

#pragma region Posture
void UBaseParryComponent::InflictParryPostureDamage(float PostureDamage)
{
	float Multiplier = PostureMultipliers[CurrentParryState];
	AddToCurrentPosture(-PostureDamage * Multiplier);
}

void UBaseParryComponent::SetMaxPosture(float Amount)
{
	MaxPosture = Amount;
}

void UBaseParryComponent::AddToCurrentPosture(float Amount)
{
	CurrentPosture = FMath::Min(CurrentPosture + Amount, MaxPosture);

	// PrintOnScreen_Local(FString::Printf(TEXT("Posture: %f / %f"), CurrentPosture, MaxPosture));
	
	// on posture broken
	if(CurrentPosture <= 0)
	{
		CurrentPosture = 0.f;
	}
	
	if(OnPostureChanged.IsBound())
	{
		OnPostureChanged.Broadcast(CurrentPosture, MaxPosture);
	}
}

void UBaseParryComponent::FullyRefillPosuture()
{
	CurrentPosture = MaxPosture;
}
#pragma endregion 
