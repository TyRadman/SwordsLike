// Fill out your copyright notice in the Description page of Project Settings.


#include "NetworkOverheadDebugger.h"

#include "Components/TextBlock.h"

void UNetworkOverheadDebugger::Setup(APawn* InParent)
{
	FString ActorName = InParent->GetActorNameOrLabel();
	FString NetworkRole;

	switch (InParent->GetLocalRole())
	{
		case ROLE_Authority:
			NetworkRole = TEXT("Authority");
			break;
		case ROLE_AutonomousProxy:
			NetworkRole = TEXT("Owner Client");
			break;
		case ROLE_SimulatedProxy:
			NetworkRole = TEXT("Simulated Client");
			break;
		default:
			NetworkRole = TEXT("Undefined");
			break;
	}

	FText Text = FText::FromString(FString::Printf(TEXT("%s\n%s"), *NetworkRole, *ActorName));
	DebugText->SetText(Text);
}
