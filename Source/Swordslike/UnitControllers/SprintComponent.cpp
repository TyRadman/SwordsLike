#include "SprintComponent.h"

#include "BaseEntityData.h"
#include "Net/UnrealNetwork.h"
#include "Player/SwordslikeCharacter.h"
#include "Swordslike/UI/HUD/MasterHUD.h"
#include "Swordslike/UI/HUD/HealthBars/PlayerHealthBar.h"
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
			SetMaxStamina(CustomCharacter->GetPlayerStats()->MaxStamina);
			FullyRefillStamina();

			if(const UMasterHUD* MasterHUD = CustomCharacter->GetMasterHUD())
			{
				if(UPlayerHealthBar* PlayerStatsHUD = MasterHUD->GetStatsHUD())
				{
					OnEntityStaminaChanged.AddUObject(PlayerStatsHUD, &UPlayerHealthBar::SetStaminaBarValue);
					PlayerStatsHUD->SetStaminaBarValue(1.f, 1.f);
				}
			}
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

void USprintComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(bIsSprinting)
	{
		AddToCurrentStamina(-DeltaTime);

		if(CurrentStamina <= 0.f)
		{
			PrintOnScreen_Local(FString::Printf(TEXT("Sprint: Stopped sprint, stamina: %f"), CurrentStamina));
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

void USprintComponent::AddToCurrentStamina(const float Amount)
{
	CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0, MaxStamina);
	OnStaminaUpdated();
}

void USprintComponent::SetMaxStamina(const float MaxAmount)
{
	MaxStamina = MaxAmount;
	OnStaminaUpdated();
}

void USprintComponent::SetCurrentStamina(const float Amount)
{
	CurrentStamina = FMath::Clamp(Amount, 0, MaxStamina);
	OnStaminaUpdated();
}

void USprintComponent::AddToMaxStamina(const float Amount)
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

void USprintComponent::StopRefill()
{
	bCanRefill = false;
	
	if(GetWorld()->GetTimerManager().IsTimerActive(RefillDelayTimer))
	{
		GetWorld()->GetTimerManager().ClearTimer(RefillDelayTimer);
	}
}

void USprintComponent::StartRefill()
{
	if(bIsSprinting)
	{
		return;
	}
	
	bCanRefill = false;
	
	GetWorld()->GetTimerManager().SetTimer(
		RefillDelayTimer,
		[this](){bCanRefill = true;},
		DelayBeforeRegeneration,
		false);
}

void USprintComponent::OnSprintStated()
{
	StopRefill();
	
	if (!HasAuthority())
	{
		Server_SetSprinting(true);
	}
	else
	{
		bIsSprinting = true;
		EntityCharacter->SetSprintSpeed();
	}
}

void USprintComponent::OnSprintEnded()
{
	StartRefill();
	
	if (!HasAuthority())
	{
		Server_SetSprinting(false);
	}
	else
	{
		bIsSprinting = false;
		EntityCharacter->ResetSpeed();
	}
}

void USprintComponent::Server_SetSprinting_Implementation(const bool bNewIsSprinting)
{
	bIsSprinting = bNewIsSprinting;
	
	if(bNewIsSprinting)
	{
		EntityCharacter->SetSprintSpeed();
	}
	else
	{
		EntityCharacter->ResetSpeed();
	}
}