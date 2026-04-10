// MainMenuGameMode.cpp

#include "MainMenuGameMode.h"
#include "MainMenuWidget.h"
#include "DeckManagerSubsystem.h"
#include "UI/DeckBuilderWidget.h"
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

void AMainMenuGameMode::ShowDeckBuilder()
{
	if (!GEngine || !GEngine->GameViewport) return;

	// 避免重複開啟
	if (DeckBuilderWidget.IsValid()) return;

	UGameInstance* GI = GetGameInstance();
	UDeckManagerSubsystem* DeckManager = GI ? GI->GetSubsystem<UDeckManagerSubsystem>() : nullptr;

	DeckBuilderWidget = SNew(SDeckBuilderWidget);
	DeckBuilderWidget->Initialize(CardDataTable, DeckManager);
	DeckBuilderWidget->OnCloseRequested.BindUObject(this, &AMainMenuGameMode::HideDeckBuilder);

	GEngine->GameViewport->AddViewportWidgetContent(
		SNew(SWeakWidget).PossiblyNullContent(DeckBuilderWidget.ToSharedRef()),
		10  // ZOrder：覆蓋在主選單之上
	);

	UE_LOG(LogTemp, Log, TEXT("DeckBuilder opened"));
}

void AMainMenuGameMode::HideDeckBuilder()
{
	if (!DeckBuilderWidget.IsValid()) return;

	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(DeckBuilderWidget.ToSharedRef());
	}

	DeckBuilderWidget.Reset();
	UE_LOG(LogTemp, Log, TEXT("DeckBuilder closed"));
}
