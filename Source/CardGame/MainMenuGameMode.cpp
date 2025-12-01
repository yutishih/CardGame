// MainMenuGameMode.cpp

#include "MainMenuGameMode.h"
#include "MainMenuWidget.h"
#include "Widgets/SWeakWidget.h"
#include "GameFramework/PlayerController.h"

AMainMenuGameMode::AMainMenuGameMode()
{
}

void AMainMenuGameMode::BeginPlay()
{
	Super::BeginPlay();
	
	CreateMainMenu();
}

void AMainMenuGameMode::CreateMainMenu()
{
	UWorld* World = GetWorld();
	if (!World) return;

	APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;

	// 顯示滑鼠游標
	PC->bShowMouseCursor = true;
	PC->SetInputMode(FInputModeUIOnly());

	// 創建主選單 Widget
	MainMenuWidget = SNew(SMainMenuWidget);

	// 添加到 Viewport
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(
			SNew(SWeakWidget).PossiblyNullContent(MainMenuWidget.ToSharedRef())
		);
		
		UE_LOG(LogTemp, Warning, TEXT("Main Menu created successfully"));
	}
}
