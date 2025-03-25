#include "SprintComponent.h"

#include "BaseEntityData.h"
#include "Net/UnrealNetwork.h"
#include "Player/SwordslikeCharacter.h"
#include "Weapons/Weapon.h"

USprintComponent::USprintComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void USprintComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USprintComponent, bIsSprinting);
}

void USprintComponent::InitEntityComponent(ACharacter* Character)
{
	if(Character)
	{
		if(ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Character))
		{
			EntityCharacter = CustomCharacter;
			CustomCharacter->OnJumped.AddUObject(this, &USprintComponent::OnJumped);
			
			SetMaxStamina(CustomCharacter->GetPlayerStats()->MaxStamina);
			FullyRefillStamina();
		}
		else
		{
			PrintOnScreen_Local(TEXT("Sprint: Swordslike character in Character"));
		}
	}
	else
	{
		PrintOnScreen_Local(TEXT("Sprint: no character passed"));
	}
}

void USprintComponent::BeginPlay()
{
	Super::BeginPlay();
}

void USprintComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(bIsSprinting)
	{
		AddToCurrentStamina(-DeltaTime);

		if(CurrentStamina == 0.f)
		{
			OnSprintEnded();
		}
	}
	else if(bCanRefill)
	{
		AddToCurrentStamina(DeltaTime * StaminaRegenerationRate);

		if(CurrentStamina >= MaxStamina)
		{
			StopRefill();
		}
	}
}

float USprintComponent::GetCurrentStamina() const
{
	return  CurrentStamina;	
}

float USprintComponent::GetMaxStamina() const
{
	return MaxStamina;
}

void USprintComponent::SetMaxStamina(float MaxAmount)
{
	MaxStamina = MaxAmount;
	OnStaminaUpdated();
}

void USprintComponent::SetCurrentStamina(float Amount)
{
	CurrentStamina = FMath::Clamp(Amount, 0, MaxStamina);
	OnStaminaUpdated();
}

void USprintComponent::AddToCurrentStamina(float Amount)
{
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0, MaxStamina);
	OnStaminaUpdated();
}

void USprintComponent::AddToMaxStamina(float Amount)
{
	MaxStamina += Amount;
	OnStaminaUpdated();
}

void USprintComponent::FullyRefillStamina()
{
	CurrentStamina = MaxStamina;
	OnStaminaUpdated();
}

void USprintComponent::OnWeaponHit(AWeapon* Weapon)
{
	if(!Weapon)
	{
		PrintOnScreen_Local(TEXT("Sprint: No Weapon passed"));
		return;
	}
	
	AddToCurrentStamina(-Weapon->StaminaPerHit);
	StartRefill();
}

void USprintComponent::OnJumped()
{
	AddToCurrentStamina(-JUMP_STAMINA_COST);
	StartRefill();
}

void USprintComponent::OnRolled()
{
	AddToCurrentStamina(-ROLL_STAMINA_COST);
	StartRefill();
}

void USprintComponent::OnStaminaUpdated()
{
	if(OnEntityStaminaChanged.IsBound())
	{
		OnEntityStaminaChanged.Broadcast(CurrentStamina, MaxStamina);
	}
}

void USprintComponent::StartRefill()
{
	StopRefill();
	
	GetWorld()->GetTimerManager().SetTimer(
		RefillDelayTimer,
		[this](){bCanRefill = true;},
		DelayBeforeRegeneration,
		false);
}

void USprintComponent::StopRefill()
{
	bCanRefill = false;
	
	if(GetWorld()->GetTimerManager().IsTimerActive(RefillDelayTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(RefillDelayTimer);
	}
}

void USprintComponent::OnSprintStated()
{
	if(bIsSprinting)
	{
		return;
	}
	
	bCanRefill = false;
    bIsSprinting = true; 
	
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_SetSprinting(true);
	}
	
	EntityCharacter.Get()->SetSprintSpeed();
}

void USprintComponent::Server_SetSprinting_Implementation(bool bNewIsSprinting)
{
	bIsSprinting = bNewIsSprinting;
}

bool USprintComponent::Server_SetSprinting_Validate(bool bNewIsSprinting)
{
	return true;
}

void USprintComponent::OnSprintEnded()
{
	if(!bIsSprinting)
	{
		return;
	}

	// start the timer to allow stamina regeneration
	StartRefill();
	
    bIsSprinting = false; 
	
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_SetSprinting(false);
	}
	
	EntityCharacter->ResetSpeed();
}