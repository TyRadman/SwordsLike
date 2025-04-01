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
	UPROPERTY(ReplicatedUsing = OnRep_CurrentParryState)
	EParryState CurrentParryState;
	UFUNCTION()
	void OnRep_CurrentParryState();


public:	
	UBaseParryComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void InitEntityComponent(ACharacter* Character) override;

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
	
	UPROPERTY(ReplicatedUsing = OnRep_MaxPosture)
	float MaxPosture;
	UFUNCTION()
	void OnRep_MaxPosture();

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
	bool bCanRecoverPosture = true;

	void PerformKnockDown();
	void RecoverFromKnockDown();
	float KnockDownRecoveryTime;
	FTimerHandle KnockDownRecoveryTimerHandle;
	

public:
	void SetMaxPosture(const float NewAmount);
	void FullyRefillPosture();
	bool bIsKnockedDown = false;
	bool bIsParrying = false;

	ParryDelegate OnParryStartedEvent;
	ParryDelegate OnParryEndedEvent;
	SuccessfulParryDelegate OnParrySuccessful_Local;

	// so far only for UI updates
	PostureDelegateTwoParams OnPostureChanged;
	PostureDelegate OnKnockedDown;
	PostureDelegate OnKnockedDownRecover;
};
