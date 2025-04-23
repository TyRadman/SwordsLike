#include "LobbyHUD.h"

#include "MainLobbyMenu.h"
#include "Blueprint/UserWidget.h"

ALobbyHUD::ALobbyHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget>LobbyWidgetClassReference(TEXT("/Game/UI/Lobby/WPB_MainLobby"));
	
	if(LobbyWidgetClassReference.Class)
	{
		LobbyUI = CreateWidget<UMainLobbyMenu>(GetWorld(), LobbyWidgetClassReference.Class);
	}
}

void ALobbyHUD::BeginPlay()
{
	Super::BeginPlay();
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ALobbyHUD: CALLED [%s]"), *UEnum::GetValueAsString(GetLocalRole())));

	if (LobbyUI)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("ALobbyHUD: created [%s]"), *UEnum::GetValueAsString(GetLocalRole())));
		LobbyUI->AddToViewport();
		LobbyUI->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "ERROR: No Lobby UI Added");
	}
}
