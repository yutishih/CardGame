// Copyright Epic Games, Inc. All Rights Reserved.

#include "CardGameSimpleHUD.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Kismet/GameplayStatics.h"

void UCardGameSimpleHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// 自動尋找 GameMode
	if (!BattleGameMode)
	{
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(GetWorld());
		BattleGameMode = Cast<ACardBattle>(GameMode);
	}
}

void UCardGameSimpleHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateDisplay();
}

void UCardGameSimpleHUD::SetBattleGameMode(ACardBattle* InBattleGameMode)
{
	BattleGameMode = InBattleGameMode;
}

TSharedRef<SWidget> UCardGameSimpleHUD::RebuildWidget()
{
	return SNew(SOverlay)
		+ SOverlay::Slot()
		.HAlign(HAlign_Fill)
		.VAlign(VAlign_Fill)
		[
			SNew(SVerticalBox)

			// 標題
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f)
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("🎴 CARD BATTLE 🎴")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 32))
				.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
			]

			// 分數區域
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 10.0f)
			[
				SNew(SHorizontalBox)

				// Player 0 分數
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Center)
				[
					SAssignNew(Player0ScoreText, STextBlock)
					.Text(FText::FromString(TEXT("Player 0: 0")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.ColorAndOpacity(FSlateColor(FLinearColor::Green))
				]

				// Player 1 分數
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.HAlign(HAlign_Center)
				[
					SAssignNew(Player1ScoreText, STextBlock)
					.Text(FText::FromString(TEXT("Player 1: 0")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.ColorAndOpacity(FSlateColor(FLinearColor::Red))
				]
			]

			// 遊戲狀態
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 10.0f)
			.HAlign(HAlign_Center)
			[
				SAssignNew(GameStateText, STextBlock)
				.Text(FText::FromString(TEXT("Game State: Idle")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 18))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]

			// 當前回合
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 5.0f)
			.HAlign(HAlign_Center)
			[
				SAssignNew(CurrentTurnText, STextBlock)
				.Text(FText::FromString(TEXT("Current Turn: -")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]

			// 計時器
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 5.0f)
			.HAlign(HAlign_Center)
			[
				SAssignNew(TimerText, STextBlock)
				.Text(FText::FromString(TEXT("Time: 3.0s")))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
				.ColorAndOpacity(FSlateColor(FLinearColor::Green))
			]

			// === 檯面區域 ===
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 20.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("=== TABLE ===")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
					.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
				]
				// 玩家已出的牌
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("YOUR PLAYED CARDS:")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FSlateColor(FLinearColor::Green))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 5.0f)
					[
						SAssignNew(Player0PlayedCardsBox, SHorizontalBox)
					]
				]
				// 對手已出的牌
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 10.0f)
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("OPPONENT PLAYED CARDS:")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
						.ColorAndOpacity(FSlateColor(FLinearColor::Red))
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 5.0f)
					[
						SAssignNew(Player1PlayedCardsBox, SHorizontalBox)
					]
				]
			]

			// 上回合結果
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 15.0f)
			.HAlign(HAlign_Center)
			[
				SAssignNew(LastRoundText, STextBlock)
				.Text(FText::FromString(TEXT("Last Round: -")))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
				.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
			]

			// 玩家 0 手牌（可點擊的按鈕）
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 10.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("YOUR HAND (Click to Play):")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FSlateColor(FLinearColor::Green))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 5.0f)
				[
					SAssignNew(Player0HandBox, SHorizontalBox)
				]
			]

			// 玩家 1 手牌
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(20.0f, 10.0f)
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Player 1 Hand:")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
					.ColorAndOpacity(FSlateColor(FLinearColor::Red))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(Player1HandText, STextBlock)
					.Text(FText::FromString(TEXT("[ ]")))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 20))
					.ColorAndOpacity(FSlateColor(FLinearColor::White))
				]
			]

			// 獲勝者顯示
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.VAlign(VAlign_Center)
			.HAlign(HAlign_Center)
			[
				SNew(SVerticalBox)
				// 獲勝文字
				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SAssignNew(WinnerText, STextBlock)
					.Text(FText::FromString(TEXT("")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 48))
					.ColorAndOpacity(FSlateColor(FLinearColor::Yellow))
				]
				// 遊戲結束選單（Play Again / Back to Menu）
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(0.0f, 20.0f)
				.HAlign(HAlign_Center)
				[
					SAssignNew(GameOverMenuBox, SVerticalBox)
					.Visibility(EVisibility::Collapsed)
					
					// Play Again 按鈕
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(10.0f)
					[
						SNew(SButton)
						.OnClicked_Lambda([this]() -> FReply
						{
							return OnPlayAgainClicked();
						})
						.ButtonColorAndOpacity(FLinearColor(0.1f, 0.5f, 0.1f, 1.0f))
						[
							SNew(SBox)
							.MinDesiredWidth(250.0f)
							.MinDesiredHeight(60.0f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Play Again")))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
								.ColorAndOpacity(FSlateColor(FLinearColor::White))
							]
						]
					]
					
					// Back to Main Menu 按鈕
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(10.0f)
					[
						SNew(SButton)
						.OnClicked_Lambda([this]() -> FReply
						{
							return OnBackToMenuClicked();
						})
						.ButtonColorAndOpacity(FLinearColor(0.5f, 0.3f, 0.1f, 1.0f))
						[
							SNew(SBox)
							.MinDesiredWidth(250.0f)
							.MinDesiredHeight(60.0f)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text(FText::FromString(TEXT("Back to Main Menu")))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
								.ColorAndOpacity(FSlateColor(FLinearColor::White))
							]
						]
					]
				]
			]
		];
}

void UCardGameSimpleHUD::UpdateDisplay()
{
	if (!BattleGameMode)
	{
		return;
	}

	// 更新分數
	if (Player0ScoreText.IsValid())
	{
		Player0ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Player 0: %d"), BattleGameMode->GetPlayerScore(0))));
	}

	if (Player1ScoreText.IsValid())
	{
		Player1ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Player 1: %d"), BattleGameMode->GetPlayerScore(1))));
	}

	// 更新遊戲狀態
	if (GameStateText.IsValid())
	{
		EBattleState State = BattleGameMode->GetBattleState();
		FString StateStr;
		switch (State)
		{
		case EBattleState::Idle: StateStr = TEXT("Idle"); break;
		case EBattleState::Started: StateStr = TEXT("Started"); break;
		case EBattleState::WaitingForPlayer0: StateStr = TEXT("YOUR TURN! Click a card!"); break;
		case EBattleState::WaitingForPlayer1: StateStr = TEXT("Opponent's Turn..."); break;
		case EBattleState::RoundEnd: StateStr = TEXT("Round End"); break;
		case EBattleState::GameOver: StateStr = TEXT("Game Over!"); break;
		default: StateStr = TEXT("Unknown"); break;
		}
		GameStateText->SetText(FText::FromString(StateStr));
		
		// 當輪到玩家時，狀態文字變成醒目的顏色
		if (State == EBattleState::WaitingForPlayer0)
		{
			GameStateText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
		}
		else
		{
			GameStateText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
		}
	}

	// 更新當前回合
	if (CurrentTurnText.IsValid())
	{
		CurrentTurnText->SetText(FText::FromString(FString::Printf(TEXT("Current Turn: Player %d"), BattleGameMode->GetCurrentTurnPlayerId())));
	}

	// 更新檯面上的牌（所有已出的牌）
	UpdatePlayedCards();

	// 更新計時器（用文字和顏色表示）
	float RemainingTime = BattleGameMode->GetRemainingTurnTime();
	if (TimerText.IsValid())
	{
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1fs"), RemainingTime)));
		
		// 根據時間改變顏色（5 秒為基準）
		float Percent = RemainingTime / 5.0f;
		if (Percent > 0.5f)
		{
			TimerText->SetColorAndOpacity(FSlateColor(FLinearColor::Green));
		}
		else if (Percent > 0.25f)
		{
			TimerText->SetColorAndOpacity(FSlateColor(FLinearColor::Yellow));
		}
		else
		{
			TimerText->SetColorAndOpacity(FSlateColor(FLinearColor::Red));
		}
	}

	// 更新上回合結果
	if (LastRoundText.IsValid())
	{
		const FRoundInfo& LastRound = BattleGameMode->GetLastRoundInfo();
		if (LastRound.Player0Card.IsValid() && LastRound.Player1Card.IsValid())
		{
			FString WinnerStr;
			if (LastRound.WinnerID == 0) WinnerStr = TEXT("YOU Win!");
			else if (LastRound.WinnerID == 1) WinnerStr = TEXT("Opponent Wins!");
			else WinnerStr = TEXT("Draw!");
			
			LastRoundText->SetText(FText::FromString(FString::Printf(
				TEXT("Last Round: You[%d] vs Opponent[%d] - %s"),
				LastRound.Player0Card.CardValue,
				LastRound.Player1Card.CardValue,
				*WinnerStr)));
		}
	}

	// 更新玩家 0 手牌按鈕
	UpdatePlayerHandButtons();

	if (Player1HandText.IsValid())
	{
		const TArray<FCard>& Hand1 = BattleGameMode->GetPlayerHand(1);
		FString HandStr;
		for (int32 i = 0; i < Hand1.Num(); ++i)
		{
			HandStr += TEXT("[?] "); // 隱藏對手手牌
		}
		if (HandStr.IsEmpty()) HandStr = TEXT("(Empty)");
		Player1HandText->SetText(FText::FromString(HandStr));
	}

	// 更新獲勝者
	if (WinnerText.IsValid())
	{
		if (BattleGameMode->GetBattleState() == EBattleState::GameOver)
		{
			int32 Winner = BattleGameMode->GetWinner();
			if (Winner == 0)
			{
				WinnerText->SetText(FText::FromString(TEXT("YOU WIN!")));
			}
			else if (Winner == 1)
			{
				WinnerText->SetText(FText::FromString(TEXT("YOU LOSE!")));
			}
			else
			{
				WinnerText->SetText(FText::FromString(TEXT("DRAW!")));
			}
		}
		else
		{
			WinnerText->SetText(FText::FromString(TEXT("")));
		}
	}
	
	// 更新遊戲結束選單
	UpdateGameOverMenu();
}

void UCardGameSimpleHUD::UpdateGameOverMenu()
{
	if (!GameOverMenuBox.IsValid() || !BattleGameMode)
	{
		return;
	}
	
	// 遊戲結束時顯示選單
	if (BattleGameMode->GetBattleState() == EBattleState::GameOver)
	{
		GameOverMenuBox->SetVisibility(EVisibility::Visible);
	}
	else
	{
		GameOverMenuBox->SetVisibility(EVisibility::Collapsed);
	}
}

void UCardGameSimpleHUD::UpdatePlayerHandButtons()
{
	if (!Player0HandBox.IsValid() || !BattleGameMode)
	{
		return;
	}

	// 獲取當前手牌
	const TArray<FCard>& Hand = BattleGameMode->GetPlayerHand(0);
	
	// 檢查是否需要更新按鈕（手牌數量變化時）
	if (Player0HandBox->GetChildren()->Num() == Hand.Num())
	{
		return; // 數量沒變，不需要重建
	}

	// 清空現有按鈕
	Player0HandBox->ClearChildren();

	// 為每張牌創建按鈕
	for (int32 i = 0; i < Hand.Num(); ++i)
	{
		const int32 CardIndex = i; // 捕獲索引
		const int32 CardValue = Hand[i].CardValue;

		Player0HandBox->AddSlot()
		.AutoWidth()
		.Padding(5.0f)
		[
			SNew(SButton)
			.OnClicked_Lambda([this, CardIndex]() -> FReply
			{
				return OnCardClicked(CardIndex);
			})
			.ButtonColorAndOpacity(FLinearColor(0.2f, 0.6f, 0.2f, 1.0f))
			[
				SNew(SBox)
				.MinDesiredWidth(60.0f)
				.MinDesiredHeight(80.0f)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("%d"), CardValue)))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.ColorAndOpacity(FSlateColor(FLinearColor::White))
				]
			]
		];
	}

	// 如果手牌為空，顯示提示
	if (Hand.Num() == 0)
	{
		Player0HandBox->AddSlot()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("(No cards left)")))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 16))
			.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
		];
	}
}

void UCardGameSimpleHUD::UpdatePlayedCards()
{
	if (!BattleGameMode)
	{
		return;
	}

	const TArray<FCard>& Player0Cards = BattleGameMode->GetPlayer0PlayedCards();
	const TArray<FCard>& Player1Cards = BattleGameMode->GetPlayer1PlayedCards();

	// 只有當牌數變化時才更新 UI
	if (Player0PlayedCardsBox.IsValid() && Player0Cards.Num() != LastPlayer0CardCount)
	{
		LastPlayer0CardCount = Player0Cards.Num();
		Player0PlayedCardsBox->ClearChildren();

		if (Player0Cards.Num() == 0)
		{
			Player0PlayedCardsBox->AddSlot()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("(No cards played yet)")))
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
			];
		}
		else
		{
			for (const FCard& Card : Player0Cards)
			{
				Player0PlayedCardsBox->AddSlot()
				.AutoWidth()
				.Padding(3.0f)
				[
					SNew(SBox)
					.MinDesiredWidth(40.0f)
					.MinDesiredHeight(50.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("[%d]"), Card.CardValue)))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FSlateColor(FLinearColor::Green))
					]
				];
			}
		}
	}

	if (Player1PlayedCardsBox.IsValid() && Player1Cards.Num() != LastPlayer1CardCount)
	{
		LastPlayer1CardCount = Player1Cards.Num();
		Player1PlayedCardsBox->ClearChildren();

		if (Player1Cards.Num() == 0)
		{
			Player1PlayedCardsBox->AddSlot()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("(No cards played yet)")))
				.Font(FCoreStyle::GetDefaultFontStyle("Italic", 14))
				.ColorAndOpacity(FSlateColor(FLinearColor::Gray))
			];
		}
		else
		{
			for (const FCard& Card : Player1Cards)
			{
				Player1PlayedCardsBox->AddSlot()
				.AutoWidth()
				.Padding(3.0f)
				[
					SNew(SBox)
					.MinDesiredWidth(40.0f)
					.MinDesiredHeight(50.0f)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(TEXT("[%d]"), Card.CardValue)))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
						.ColorAndOpacity(FSlateColor(FLinearColor::Red))
					]
				];
			}
		}
	}
}

FReply UCardGameSimpleHUD::OnCardClicked(int32 CardIndex)
{
	if (BattleGameMode)
	{
		// 只有在玩家 0 的回合才能出牌
		if (BattleGameMode->GetBattleState() == EBattleState::WaitingForPlayer0)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player clicked card at index %d"), CardIndex);
			BattleGameMode->PlayerPlayCard(0, CardIndex);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Not your turn!"));
		}
	}
	return FReply::Handled();
}

FReply UCardGameSimpleHUD::OnPlayAgainClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Play Again clicked!"));
	
	// 重新載入當前關卡
	UWorld* World = GetWorld();
	if (World)
	{
		FString CurrentLevel = World->GetMapName();
		CurrentLevel.RemoveFromStart(World->StreamingLevelsPrefix);
		UGameplayStatics::OpenLevel(World, FName(*CurrentLevel));
	}
	
	return FReply::Handled();
}

FReply UCardGameSimpleHUD::OnBackToMenuClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Back to Main Menu clicked!"));
	
	// 載入主選單關卡（假設主選單是 MainMenu 或第一個關卡）
	UWorld* World = GetWorld();
	if (World)
	{
		// 嘗試載入 MainMenu 關卡，如果沒有就載入 TheFirstMap
		UGameplayStatics::OpenLevel(World, FName(TEXT("MainMenu")));
	}
	
	return FReply::Handled();
}
