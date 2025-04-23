#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "Common/DamageInfo.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "BaseParryComponent.generated.h"

class UBaseEntityAnimationsComponent;

UENUM(BlueprintType)
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
	
	UFUNCTION(Server, Reliable)
	void Server_InterruptAttacker(ASwordslikeCharacter* Attacker);

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
	
	void PlayParryEffects(const FDamageInfo DamageInfo);
	UFUNCTION(Server, Reliable)
	void Server_PlayParryEffects(const FDamageInfo DamageInfo);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayParryEffects(const FDamageInfo DamageInfo);

	void RecievePostureDamage(const FDamageInfo DamageInfo);
	UFUNCTION(Client, Reliable)
	void Client_RecievePostureDamage(const FDamageInfo DamageInfo);
	UFUNCTION(Server, Reliable)
	void Server_RecievePostureDamage(const FDamageInfo DamageInfo);
	
	void DamagePosture(const FDamageInfo DamageInfo);
	UFUNCTION(Server, Reliable)
	void Server_DamagePosture(const FDamageInfo DamageInfo);
	UFUNCTION(Client, Reliable)
	void Client_DamagePosture(const FDamageInfo DamageInfo);
	void PerformDamagePosture(const FDamageInfo DamageInfo);
	
	void OnParry();
	
	void AddToCurrentPosture(const FDamageInfo DamageInfo);
	UFUNCTION(Server, Reliable)
	void Server_AddToCurrentPosture(const FDamageInfo DamageInfo, const ECombatState State);
	void PerformServerAddToCurrentPosture(const FDamageInfo DamageInfo, const ECombatState State);
	void PerformAddToCurrentPosture(const float Amount);
	
	const float PostureRecoveryRate = 0.5f;
	const float DelayBeforePostureRecovery = 1.f;
	FTimerHandle PostureRecoveryTimerHandle;
	
	// UPROPERTY(Replicated)
	bool bCanRecoverPosture = true;

	void PerformStun();

	UFUNCTION(Server, Reliable)
	void Server_PerformRecoverFromStun();
	
	void PerformRecoverFromStun();
	float StunRecoveryTime = 2.f;
	FTimerHandle RecoveryTimerHandle;

	
	void OnKnockDown(AActor* Attacker);
	UFUNCTION(Server, Reliable)
	void Server_PerformKnockDown();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PerformKnockDown(AActor* Attacker);
	void PerformKnockDown(const AActor* Attacker);
	float KnockDownRecoveryTime;
	
	ECombatState CurrentCombatState = ECombatState::Normal;
	UBaseEntityAnimationsComponent* AnimationComponent;

	FDamageInfo RecoverDamageInfo;

public:
	void OnStunned();
	void OnRecoverFromStun();
	void StartRecoveryFromKnockDown();
	void EndRecoveryFromKnockDown();

	// UPROPERTY(ReplicatedUsing = OnRep_CurrentCombatState)
	void SetCombatState(const ECombatState State);
	UFUNCTION(Client, Reliable)
	void Client_SetCombatState(const ECombatState State);
	
	void SetMaxPosture(const float NewAmount);
	void FullyRefillPosture();
	bool bIsParrying = false;

	ParryDelegate OnParryStartedEvent;
	ParryDelegate OnParryEndedEvent;
	/**
	 * Performs non-gameplay crucial tasks like playing parry particles.
	 */
	SuccessfulParryDelegate OnParrySuccessful_Local;

	// so far only for UI updates
	PostureDelegateTwoParams OnPostureChanged;
	
	void SetParryState(const EParryState State);
	UFUNCTION(Server, Reliable)
	void Server_SetParryState(const EParryState State);
	void PerformSetParryState(const EParryState State);

private:
	ASwordslikeCharacter* PlayerCharacter;
	// TODO: Remove this if proved to be irrelevant.
	AController* OwnerController;
	UAnimInstance* AnimInstance;

	bool PendingPostureBreak = false;
	bool bIsLocallyControlled = false;
	
	UPROPERTY(EditDefaultsOnly, Category=CameraShake, meta=(AllowPrivateAccess=true))
	TSubclassOf<UCameraShakeBase> StunCameraShake;
	UPROPERTY(EditDefaultsOnly, Category=CameraShake, meta=(AllowPrivateAccess=true))
	TSubclassOf<UCameraShakeBase> KnockDownCameraShake;
};
