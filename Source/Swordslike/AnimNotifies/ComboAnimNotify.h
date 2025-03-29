#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ComboAnimNotify.generated.h"

UENUM(BlueprintType)
enum class ECombatNotifyType : uint8
{
	Damage UMETA(DisplayName = "Damage"),
	Input UMETA(DisplayName = "Allow Input"),
	PerformNextAttack UMETA(DisplayName = "Perform Next Attack"),
	IndicatorTime UMETA(DisplayName = "Indicator Time")
};

UENUM(BlueprintType)
enum class EHitType : uint8
{
	SmallHite UMETA(DisplayName = "Small hit"),
	BigHite UMETA(DisplayName = "Big hit"),
	KnockOutHit UMETA(DisplayName = "Knock out hit")
};

/**
 * An Anim Notify that is used in combo animations to read the player's input and determine whether to continue the combo or stop.
 */
UCLASS()
class SWORDSLIKE_API UComboAnimNotify : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	ECombatNotifyType NotifyType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FName NotifyName;

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

	// Attack vars
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (EditCondition = "NotifyType == ECombatNotifyType::Damage"))
	float Damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (EditCondition = "NotifyType == ECombatNotifyType::Damage"))
	EHitType HitType; 
};
