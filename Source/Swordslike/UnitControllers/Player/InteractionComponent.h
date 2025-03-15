#pragma once

#include "CoreMinimal.h"
#include "IEntityComponent.h"
#include "SwordslikeCharacter.h"
#include "ViewportInteractionTypes.h"
#include "Components/ActorComponent.h"
#include "Swordslike/Combat/Interactable.h"
#include "Swordslike/Core/MyActorComponent.h"
#include "InteractionComponent.generated.h"


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
	
	void Interact();
	
	UFUNCTION(Server, Reliable)
	void Server_Interact();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Interact();

	InteractionEventOneParam OnInteractableOverlapStarted;
	InteractionEvent OnInteractableOverlapEnded;

	FORCEINLINE IInteractable* GetCurrentInteractable() const { return CurrentInteractable; }
	
	IInteractable* CurrentInteractable;

private:
	UCapsuleComponent* SphereComponent;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	ASwordslikeCharacter* OwnerCharacter;

};
