#include "BaseHealthComponent.h"

#include <string>

#include "DamageInfo.h"
#include "Net/UnrealNetwork.h"

UBaseHealthComponent::UBaseHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBaseHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBaseHealthComponent, bIsInvincible);
	DOREPLIFETIME(UBaseHealthComponent, bIsAlive);
}

void UBaseHealthComponent::BeginPlay()
{
	Super::BeginPlay();
}

#pragma region Network Functions
#pragma region Take Damage
void UBaseHealthComponent::TakeDamage(const FDamageInfo& Info)
{
	if(bIsInvincible)
	{
		return;
	}

	if(!HasAuthority())
	{
		ServerTakeDamage(Info);
	}
}

void UBaseHealthComponent::ServerTakeDamage_Implementation(const FDamageInfo& Info)
{
	if(bIsInvincible)
	{
		return;
	}
	
	// PrintOnScreen(FString::Printf(TEXT("SERVER1: %s ATTACKED %s"), *Info.Instigator->GetActorNameOrLabel(), *GetOwner()->GetActorNameOrLabel()));
	ApplyDamage(Info);
	MulticastTakeDamage(Info);
}

void UBaseHealthComponent::MulticastTakeDamage_Implementation(const FDamageInfo& Info)
{
	if(!HasAuthority())
	{
		ApplyDamage(Info);
	}
}
#pragma endregion

#pragma region Set Max Health
void UBaseHealthComponent::SetMaxHealth(float MaxHP)
{
	// PrintOnScreen_Local(FString::Printf(TEXT("CLIENT Max Health: %f"), MaxHP), 20.f);
	
	ApplyMaxHealth(MaxHP);
	
	if(GetOwner()->GetLocalRole() < ROLE_Authority)
	{
		ServerSetMaxHealth(MaxHP);
	}
}

float UBaseHealthComponent::GetMaxHealth()
{
	return MaxHealth;
}

void UBaseHealthComponent::ServerSetMaxHealth_Implementation(float MaxHP)
{
	if (GetOwner()->GetLocalRole() == ROLE_Authority && GetOwnerRole() != ROLE_AutonomousProxy)
	{
		MulticastSetMaxHealth(MaxHP);
	}
}

void UBaseHealthComponent::MulticastSetMaxHealth_Implementation(float MaxHP)
{
	ApplyMaxHealth(MaxHP);
}
#pragma endregion 
#pragma endregion Network Functions

#pragma region Utilities
void UBaseHealthComponent::ApplyDamage(const FDamageInfo& DamageInfo)
{
	OnEntityHit.Broadcast(DamageInfo);
}

void UBaseHealthComponent::AddToCurrentHealth(const FDamageInfo& DamageInfo)
{
	CurrentHealth = FMath::Clamp(CurrentHealth - DamageInfo.Damage, 0.0f, MaxHealth);
	OnEntityHealthChanged.Broadcast(CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0)
	{
		// PrintOnScreen_Local(FString::Printf(TEXT("CLIENT (DEATH): %f / %f"), CurrentHealth, MaxHealth));
		bIsAlive = false;
		OnEntityDeath.Broadcast(DamageInfo);
	}
}

void UBaseHealthComponent::FullyChargeHealth()
{
	CurrentHealth = MaxHealth;
	
	// PrintOnScreen_Local(FString::Printf(TEXT("CLIENT Starting Health: %f / %f"), CurrentHealth, MaxHealth), 20.f);
	OnEntityHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UBaseHealthComponent::OnDeath()
{
	
}

bool UBaseHealthComponent::IsAlive()
{
	return bIsAlive;
}

void UBaseHealthComponent::ApplyMaxHealth(float MaxHP)
{
	float HealthPercentage = CurrentHealth / MaxHealth;
	MaxHealth = MaxHP;
	CurrentHealth = MaxHealth * HealthPercentage;
	OnEntityHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UBaseHealthComponent::SetIsInvincible(bool IsInvincible)
{
	bIsInvincible = IsInvincible;
}

bool UBaseHealthComponent::IsInvincible() const
{
	return bIsInvincible;
}
#pragma endregion Utilities