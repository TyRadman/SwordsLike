#include "InteractionComponent.h"

#include "SwordslikeCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Swordslike/Combat/Interactable.h"
#include "Swordslike/UI/HUD/InteractionPanel.h"
#include "Swordslike/UI/HUD/MasterHUD.h"

UInteractionComponent::UInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
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
		
		if(OwnerCharacter ->GetInteractionSphere())
		{
			SphereComponent = OwnerCharacter ->GetInteractionSphere();
			SphereComponent->SetGenerateOverlapEvents(true);
			SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			
			SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &UInteractionComponent::OnOverlapBegin);
			SphereComponent->OnComponentEndOverlap.AddDynamic(this, &UInteractionComponent::OnOverlapEnd);
			PrintOnScreen(TEXT("UInteractionComponent: Setup"));

			if(OwnerCharacter ->GetMasterHUD())
			{
				if(UInteractionPanel* InteractionHUD = OwnerCharacter ->GetMasterHUD()->GetInteractionPanel())
				{
					OnInteractableOverlapStarted.AddUObject(InteractionHUD, &UInteractionPanel::DisplayInteractionPanel);
					OnInteractableOverlapEnded.AddUObject(InteractionHUD, &UInteractionPanel::HideInteractionPanel);
				}
				else
				{
					PrintOnScreen_Local(TEXT("UInteractionComponent: No interaction HUD"));
				}
			}
			else
			{
					PrintOnScreen_Local(TEXT("UInteractionComponent: No Master HUD"));
			}
		}
		else
		{
			PrintOnScreen_Local(TEXT("UInteractionComponent: No Sphere Component"));
		}
	}
	else
	{
		PrintOnScreen_Local(TEXT("UInteractionComponent: No Custom Character"));
	}
}

void UInteractionComponent::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OtherActor && OwnerCharacter)
	{
		if(IInteractable* Interactable = Cast<IInteractable>(OtherActor))
		{
			CurrentInteractable = Interactable;

			if(OnInteractableOverlapStarted.IsBound())
			{
				OnInteractableOverlapStarted.Broadcast(OwnerCharacter );
			}
		}
	}
	else
	{
		PrintOnScreen(TEXT("INTERACTED"));
	}
}

void UInteractionComponent::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	CurrentInteractable = nullptr;
	
	if(OnInteractableOverlapEnded.IsBound())
	{
		OnInteractableOverlapEnded.Broadcast();
	}
}

void UInteractionComponent::Interact()
{
	// PrintOnScreen_Local(TEXT("Interact 1"));
	
	if(!GetOwner()->HasAuthority())
	{
		// PrintOnScreen_Local(TEXT("Interact 2"));
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
		CurrentInteractable->Interact(GetOwner());
	}
}

