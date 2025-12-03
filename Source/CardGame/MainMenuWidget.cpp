// MainMenuWidget.cpp

#include "MainMenuWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void SMainMenuWidget::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		// 全螢幕覆蓋
		SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			// 主選單容器
			SNew(SBox)
			.WidthOverride(400)
			.HeightOverride(500)
			[
				SNew(SVerticalBox)
				
				// 標題
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(20, 40, 20, 60)
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("CARD GAME")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
					.ColorAndOpacity(FSlateColor(FLinearColor::White))
				]
				
				// Start 按鈕
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(50, 10)
				.HAlign(HAlign_Fill)
				[
					CreateMenuButton(
						FText::FromString(TEXT("Start")),
						FOnClicked::CreateSP(this, &SMainMenuWidget::OnStartClicked)
					)
				]
				
				// Deck 按鈕
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(50, 10)
				.HAlign(HAlign_Fill)
				[
					CreateMenuButton(
						FText::FromString(TEXT("Deck")),
						FOnClicked::CreateSP(this, &SMainMenuWidget::OnDeckClicked)
					)
				]
				
				// Quit 按鈕
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(50, 10)
				.HAlign(HAlign_Fill)
				[
					CreateMenuButton(
						FText::FromString(TEXT("Quit")),
						FOnClicked::CreateSP(this, &SMainMenuWidget::OnQuitClicked)
					)
				]
			]
		]
	];
}

TSharedRef<SWidget> SMainMenuWidget::CreateMenuButton(const FText& ButtonText, FOnClicked OnClicked)
{
	return SNew(SButton)
		.OnClicked(OnClicked)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.HeightOverride(60)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(ButtonText)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor::Black))
			]
		];
}

FReply SMainMenuWidget::OnStartClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Start button clicked - Loading game map"));
	
	// 載入遊戲地圖
	UWorld* World = nullptr;
	
	// 嘗試多種方式獲取 World
	if (GEngine)
	{
		World = GEngine->GetCurrentPlayWorld();
		
		if (!World)
		{
			// 備用方法：從 GameViewport 獲取
			if (GEngine->GameViewport)
			{
				World = GEngine->GameViewport->GetWorld();
			}
		}
	}
	
	if (World)
	{
		UE_LOG(LogTemp, Warning, TEXT("World found, opening TheFirstMap"));
		UGameplayStatics::OpenLevel(World, FName(TEXT("TheFirstMap")));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get World reference!"));
	}
	
	return FReply::Handled();
}

FReply SMainMenuWidget::OnDeckClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Deck button clicked - Feature coming soon"));
	
	// TODO: 實作套牌編輯功能
	// 目前只顯示訊息
	
	return FReply::Handled();
}

FReply SMainMenuWidget::OnQuitClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Quit button clicked - Exiting game"));
	
	// 退出遊戲
	UWorld* World = nullptr;
	
	if (GEngine)
	{
		World = GEngine->GetCurrentPlayWorld();
		
		if (!World && GEngine->GameViewport)
		{
			World = GEngine->GameViewport->GetWorld();
		}
	}
	
	if (World)
	{
		APlayerController* PC = World->GetFirstPlayerController();
		if (PC)
		{
			UKismetSystemLibrary::QuitGame(World, PC, EQuitPreference::Quit, false);
		}
	}
	
	// 如果上面方法失敗，強制請求退出
	if (GEngine)
	{
		GEngine->Exec(nullptr, TEXT("QUIT"));
	}
	
	return FReply::Handled();
}
