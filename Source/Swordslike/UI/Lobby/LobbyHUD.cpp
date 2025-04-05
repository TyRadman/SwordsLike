#include "LobbyHUD.h"

#include "MainLobbyMenu.h"
#include "Blueprint/UserWidget.h"

ALobbyHUD::ALobbyHUD()
{
	static ConstructorHelpers::FClassFinder<UUserWidget>LobbyWidgetClass(TEXT("/Game/UI/Lobby/WPB_MainLobby"));

	if(LobbyWidgetClass.Class)
	{
		LobbyUI = CreateWidget<UMainLobbyMenu>(GetWorld(), LobbyWidgetClass.Class);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Red, TEXT("NO LOBBY WIDGET CLASS"));
	}
}

void ALobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	if (LobbyUI)
	{
		LobbyUI->AddToViewport();
		LobbyUI->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "ERROR: No Lobby UI Added");
	}
}
