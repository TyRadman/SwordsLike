#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Common/DamageInfo.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "BaseParryComponent.generated.h"

UENUM()
enum class EParryState : uint8
{
	None = 0,
	Perfect = 1,
	Good = 2,
	Normal = 3
};

UENUM()
enum class ECombatState : uint8
{
	Normal = 0,
	Stunned = 1,
	KnockedDown = 2
};

USTRUCT(BlueprintType)
struct FParryDataContainer
{
	GENERATED_BODY()

	float CurrentPosture;
	
	float MaxPosture;

	ECombatState CombatState;

	AActor* LastAttacker;
};

class ASwordslikeCharacter;

DECLARE_MULTICAST_DELEGATE(ParryDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(SuccessfulParryDelegate, EParryState State);
DECLARE_MULTICAST_DELEGATE_TwoParams(PostureDelegateTwoParams, float Current, float Max);
DECLARE_MULTICAST_DELEGATE(PostureDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API UBaseParryComponent : public UMyActorComponent, public IIEntityComponent
{
	GENERATED_BODY()

	friend class ASwordslikeCharacter;

protected:
	UPROPERTY(Replicated)
	EParryState CurrentParryState;


public:	
	UBaseParryComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void InitEntityComponent(ACharacter* Character) override;
	void CacheValues();
	
	virtual void Parry();
	virtual void EndParry();

	/**
	 * Determines whether a successful parry took place when a hit is received.
	 * @param DamageInfo The damage info of the hit. 
	 * @return The current parry state.
	 */
	EParryState ValidateParry(const FDamageInfo& DamageInfo);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UAnimMontage* ParryMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UAnimMontage* StunMontage;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	UAnimMontage* KnockDownMontage;

	UFUNCTION(Server, Reliable)
	void Server_Parry();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Parry();
	UFUNCTION(Server, Reliable)
	void Server_EndParry();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EndParry();
	
	UFUNCTION()
	virtual void OnParryNotifyStart(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	const TMap<FName, EParryState> ParryStates =
		{
		{"PerfectParry", EParryState::Perfect},
		{"GoodParry", EParryState::Good},
		{"NormalParry", EParryState::Normal}
	};
	
	const TMap<EParryState, float> PostureMultipliers =
		{
		{EParryState::Perfect, 0.f},
		{EParryState::Good, 0.4f},
		{EParryState::Normal, 0.7f},
		{EParryState::None, 1.f}
		};

	const FName StartSectionName = FName("Start");
	const FName MiddleSectionName = FName("Middle");
	const FName EndSectionName = FName("End");
	const FName HitSectionName = FName("Hit");

	//////////////
	// POSTURE
	//////////////
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPosture)
	float CurrentPosture;
	UFUNCTION()
	void OnRep_CurrentPosture();
	UPROPERTY(Replicated)
	float MaxPosture;

	void DamagePosture(const FDamageInfo DamageInfo);
	UFUNCTION(Server, Reliable)
	void Server_DamagePosture(const FDamageInfo DamageInfo);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DamagePosture(const FDamageInfo DamageInfo);
	void PerformDamagePosture(const FDamageInfo DamageInfo);
	
	void OnParry();
	
	void AddToCurrentPosture(const float Amount);
	UFUNCTION(Server, Reliable)
	void Server_AddToCurrentPosture(const float Amount);
	void PerformAddToCurrentPosture(const float Amount);
	
	const float PostureRecoveryRate = 0.5f;
	const float DelayBeforePostureRecovery = 1.f;
	FTimerHandle PostureRecoveryTimerHandle;
	
	UPROPERTY(Replicated)
	bool bCanRecoverPosture = true;

	UFUNCTION(Server, Reliable)
	void Server_PerformStun();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PerformStun();
	void PerformStun();

	UFUNCTION(Server, Reliable)
	void Server_PerformRecoverFromStun();
	
	void PerformRecoverFromStun();
	float StunRecoveryTime = 2.f;
	FTimerHandle RecoveryTimerHandle;

	
	void OnKnockDown();
	UFUNCTION(Server, Reliable)
	void Server_PerformKnockDown();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PerformKnockDown();
	void PerformKnockDown();
	float KnockDownRecoveryTime;

public:
	void OnStunned();
	void OnRecoverFromStun();
	void StartRecoveryFromKnockDown();
	void EndRecoveryFromKnockDown();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentCombatState)
	ECombatState CurrentCombatState = ECombatState::Normal;
	UFUNCTION()
	void OnRep_CurrentCombatState();
	
	void SetMaxPosture(const float NewAmount);
	void FullyRefillPosture();
	bool bIsParrying = false;

	ParryDelegate OnParryStartedEvent;
	ParryDelegate OnParryEndedEvent;
	SuccessfulParryDelegate OnParrySuccessful_Local;

	// so far only for UI updates
	PostureDelegateTwoParams OnPostureChanged;

	// UPROPERTY(ReplicatedUsing=OnRep_DataContainer)
	// FParryDataContainer DataContainer;
	// UFUNCTION()
	// void OnRep_DataContainer();

private:
	ASwordslikeCharacter* PlayerCharacter;
	AController* OwnerController;
	UAnimInstance* AnimInstance;
	UPROPERTY(Replicated)
	AActor* CurrentAttacker;

	bool PendingPostureBreak = false;
};
