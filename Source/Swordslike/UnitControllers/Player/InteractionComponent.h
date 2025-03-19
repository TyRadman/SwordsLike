#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "SwordslikeCharacter.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Combat/Interactable.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "InteractionComponent.generated.h"


class AWeapon;
class UCapsuleComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(InteractionEventOneParam, ASwordslikeCharacter* Character);
DECLARE_MULTICAST_DELEGATE(InteractionEvent);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SWORDSLIKE_API UInteractionComponent : public UMyActorComponent, public IIEntityComponent
{
	GENERATED_BODY()

public:	
	UInteractionComponent();
	
	virtual void InitEntityComponent(ACharacter* Character) override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	
	void Interact();
	
	UFUNCTION(Server, Reliable)
	void Server_Interact();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Interact();

	InteractionEventOneParam OnInteractableOverlapStarted;
	InteractionEvent OnInteractableOverlapEnded;

	FORCEINLINE IInteractable* GetCurrentInteractable() const
	{
		if(IInteractable* Interactable = Cast<IInteractable>(CurrentInteractable))
		{
			return Interactable;
		}
		return nullptr;
	}

private:
	UCapsuleComponent* SphereComponent;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(Client, Reliable)
	void Client_OnOverlapEvent();
	
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	ASwordslikeCharacter* OwnerCharacter;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentInteractable)
	AActor* CurrentInteractable;

	UFUNCTION()
	void OnRep_CurrentInteractable();

};
