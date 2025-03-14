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

DECLARE_MULTICAST_DELEGATE(ParryDelegate);
DECLARE_MULTICAST_DELEGATE_TwoParams(SuccessfulParryDelegate, const FDamageInfo& DamageInfo, EParryState State);
DECLARE_MULTICAST_DELEGATE_TwoParams(PostureDelegateTwoParams, float Current, float Max);
DECLARE_MULTICAST_DELEGATE(PostureDelegate);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API UBaseParryComponent : public UMyActorComponent, public IIEntityComponent
{
	GENERATED_BODY()

protected:
	EParryState CurrentParryState;

public:	
	UBaseParryComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void InitEntityComponent(ACharacter* Character) override;

	virtual void Parry();
	virtual void EndParry();
	void DamagePosture(FDamageInfo DamageInfo);

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
	UAnimMontage* KnockDownMontage;

	ACharacter* OwnerCharacter;
	UAnimInstance* AnimInstance;

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
		{EParryState::Good, 0.5f},
		{EParryState::Normal, 0.7f},
		{EParryState::None, 1.f}
		};

	const FName StartSectionName = FName("Start");
	const FName MiddleSectionName = FName("Middle");
	const FName EndSectionName = FName("End");
	const FName HitSectionName = FName("Hit");

	// POSTURE
	UPROPERTY(ReplicatedUsing = OnRep_CurrentPosture)
	float CurrentPosture;

	UFUNCTION()
	void OnRep_CurrentPosture();

	UFUNCTION(Server, Reliable)
	void Server_AddToCurrentPosture(float Amount);
	
	float MaxPosture;

	const float PostureRecoveryRate = 0.5f;
	const float DelayBeforePostureRecovery = 1.f;
	FTimerHandle PostureRecoveryTimerHandle;
	bool bCanRecoverPosture = true;

	void PerformKnockDown();
	void RecoverFromKnockDown();
	float KnockDownRecoveryTime;
	FTimerHandle KnockDownRecoveryTimerHandle;
	

public:
	void SetMaxPosture(float MaxPosture);
	void AddToCurrentPosture(float Amount);
	void FullyRefillPosture();
	bool bIsKnockedDown = false;
	bool bIsParrying = false;

	ParryDelegate OnParryStartedEvent;
	ParryDelegate OnParryEndedEvent;
	SuccessfulParryDelegate OnParrySuccessful;
	
	PostureDelegateTwoParams OnPostureChanged;
	PostureDelegate OnKnockedDown;
	PostureDelegate OnKnockedDownRecover;
};
