#include "InteractionComponent.h"

#include "SwordslikeCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "Swordslike/Combat/Interactable.h"
#include "Swordslike/UI/HUD/InteractionPanel.h"
#include "Swordslike/UI/HUD/MasterHUD.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractionComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UInteractionComponent, CurrentInteractable, COND_OwnerOnly);
}

void UInteractionComponent::InitEntityComponent(ACharacter* Character)
{
	if(!Character)
	{
		PrintOnScreen_Local(TEXT("UInteractionComponent: No Character passed"));
		return;
	}

	if(ASwordslikeCharacter* CustomCharacter = Cast<ASwordslikeCharacter>(Character))
	{
		OwnerCharacter = CustomCharacter;
		
		if(OwnerCharacter->GetInteractionSphere())
		{
			SphereComponent = OwnerCharacter->GetInteractionSphere();
			SphereComponent->SetGenerateOverlapEvents(true);
			SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			
			SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &UInteractionComponent::OnOverlapBegin);
			SphereComponent->OnComponentEndOverlap.AddDynamic(this, &UInteractionComponent::OnOverlapEnd);

			if(OwnerCharacter ->GetMasterHUD())
			{
				if(UInteractionPanel* InteractionHUD = OwnerCharacter->GetMasterHUD()->GetInteractionPanel())
				{
					OnInteractableOverlapStarted.AddUObject(InteractionHUD, &UInteractionPanel::DisplayInteractionPanel);
					OnInteractableOverlapEnded.AddUObject(InteractionHUD, &UInteractionPanel::HideInteractionPanel);
				}
				else
				{
					PrintOnScreen_Local(TEXT("UInteractionComponent: No interaction HUD"), FColor::Purple, 20.f);
				}
			}
			else
			{
				PrintOnScreen_Local(TEXT("UInteractionComponent: No Master HUD"), FColor::Purple, 20.f);
			}
		}
		else
		{
			PrintOnScreen_Local(TEXT("UInteractionComponent: No Sphere Component"), FColor::Purple, 20.f);
		}
	}
	else
	{
		PrintOnScreen_Local(TEXT("UInteractionComponent: No Custom Character"), FColor::Purple, 20.f);
	}
}

void UInteractionComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                           UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor && OwnerCharacter)
	{
		if(Cast<IInteractable>(OtherActor))
		{
			CurrentInteractable = OtherActor;
			Client_OnOverlapEvent_Implementation();
		}
	}
	else
	{
		PrintOnScreen(TEXT("NOT INTERACTED"));
	}
}

void UInteractionComponent::OnRep_CurrentInteractable()
{
	Client_OnOverlapEvent();
}

void UInteractionComponent::Client_OnOverlapEvent_Implementation()
{
	if (OwnerCharacter)
	{
		if(CurrentInteractable)
		{
			OnInteractableOverlapStarted.Broadcast(OwnerCharacter);
		}
		else
		{
			OnInteractableOverlapEnded.Broadcast();
		}
	}
}

void UInteractionComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CurrentInteractable = nullptr;
	Client_OnOverlapEvent_Implementation();
}

void UInteractionComponent::Interact()
{
	if(!GetOwner()->HasAuthority())
	{
		Server_Interact();
	}

	Multicast_Interact_Implementation();
}

void UInteractionComponent::Server_Interact_Implementation()
{
	Multicast_Interact();
}

void UInteractionComponent::Multicast_Interact_Implementation()
{
	if(CurrentInteractable)
	{
		GetCurrentInteractable()->Interact(GetOwner());
	}
}
