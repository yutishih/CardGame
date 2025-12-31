// Copyright Epic Games, Inc. All Rights Reserved.

#include "CardGameHUD.h"
#include "UI/CardWidget.h"
#include "Data/DT_CardData.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ProgressBar.h"
#include "Kismet/GameplayStatics.h"

void UCardGameHUD::NativeConstruct()
{
	Super::NativeConstruct();

	// 自動尋找 GameMode
	if (!BattleGameMode)
	{
		AGameModeBase* GameMode = UGameplayStatics::GetGameMode(GetWorld());
		BattleGameMode = Cast<ACardBattle>(GameMode);
	}
}

void UCardGameHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 每幀更新 UI
	UpdateUI();
}

void UCardGameHUD::InitializeHUD(ACardBattle* InBattleGameMode)
{
	BattleGameMode = InBattleGameMode;
	UpdateUI();
}

void UCardGameHUD::UpdateUI()
{
	if (!BattleGameMode)
	{
		return;
	}

	// 更新分數
	if (Player0ScoreText)
	{
		Player0ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Player 0: %d"), BattleGameMode->GetPlayerScore(0))));
	}

	if (Player1ScoreText)
	{
		Player1ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Player 1: %d"), BattleGameMode->GetPlayerScore(1))));
	}

	// 更新當前回合
	if (CurrentTurnText)
	{
		int32 CurrentPlayer = BattleGameMode->GetCurrentTurnPlayerId();
		CurrentTurnText->SetText(FText::FromString(FString::Printf(TEXT("Current Turn: Player %d"), CurrentPlayer)));
	}

	// 更新計時器
	float RemainingTime = BattleGameMode->GetRemainingTurnTime();
	if (TimerText)
	{
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("Time: %.1f"), RemainingTime)));
	}

	if (TimerProgressBar)
	{
		// 假設 30 秒為滿
		TimerProgressBar->SetPercent(RemainingTime / 30.0f);
	}

	// 更新遊戲狀態
	if (GameStateText)
	{
		EBattleState State = BattleGameMode->GetBattleState();
		GameStateText->SetText(FText::FromString(GetBattleStateString(State)));
	}

	// 更新上回合結果
	if (LastRoundResultText)
	{
		const FRoundInfo& LastRound = BattleGameMode->GetLastRoundInfo();
		if (LastRound.Player0Card.IsValid() && LastRound.Player1Card.IsValid())
		{
			FString ResultStr;
			if (LastRound.WinnerID == 0)
			{
				ResultStr = FString::Printf(TEXT("Last Round: P0(%d) vs P1(%d) - Player 0 Wins!"), 
					LastRound.Player0Card.CardValue, LastRound.Player1Card.CardValue);
			}
			else if (LastRound.WinnerID == 1)
			{
				ResultStr = FString::Printf(TEXT("Last Round: P0(%d) vs P1(%d) - Player 1 Wins!"), 
					LastRound.Player0Card.CardValue, LastRound.Player1Card.CardValue);
			}
			else
			{
				ResultStr = FString::Printf(TEXT("Last Round: P0(%d) vs P1(%d) - Draw!"), 
					LastRound.Player0Card.CardValue, LastRound.Player1Card.CardValue);
			}
			LastRoundResultText->SetText(FText::FromString(ResultStr));
		}
		else
		{
			LastRoundResultText->SetText(FText::FromString(TEXT("Last Round: -")));
		}
	}

	// 更新獲勝者顯示
	if (WinnerText)
	{
		if (BattleGameMode->GetBattleState() == EBattleState::GameOver)
		{
			int32 Winner = BattleGameMode->GetWinner();
			if (Winner == 0)
			{
				WinnerText->SetText(FText::FromString(TEXT("🎉 PLAYER 0 WINS! 🎉")));
			}
			else if (Winner == 1)
			{
				WinnerText->SetText(FText::FromString(TEXT("🎉 PLAYER 1 WINS! 🎉")));
			}
			else
			{
				WinnerText->SetText(FText::FromString(TEXT("🤝 DRAW! 🤝")));
			}
			WinnerText->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			WinnerText->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 更新手牌顯示（簡化版：顯示手牌數量）
	if (Player0HandBox)
	{
		UpdatePlayerHand(0, Player0HandBox);
	}

	if (Player1HandBox)
	{
		UpdatePlayerHand(1, Player1HandBox);
	}
}

void UCardGameHUD::OnCardClicked(int32 CardIndex)
{
	if (BattleGameMode)
	{
		// 假設玩家 0 是人類玩家
		BattleGameMode->PlayerPlayCard(0, CardIndex);
	}
}

void UCardGameHUD::UpdatePlayerHand(int32 PlayerId, UHorizontalBox* HandBox)
{
	if (!HandBox || !BattleGameMode)
	{
		return;
	}

	// 清空現有內容
	HandBox->ClearChildren();

	// 獲取玩家手牌
	const TArray<FCard>& Hand = BattleGameMode->GetPlayerHand(PlayerId);

	// 檢查手牌數量是否與子元件數量一致，如果不一致才重建
	// 這樣可以避免每幀都刪除重建，造成閃爍和效能問題
	// 但為了簡單起見，這裡先保持重建邏輯，但加上 Log 檢查
	// UE_LOG(LogTemp, Warning, TEXT("Updating Hand for Player %d, Count: %d"), PlayerId, Hand.Num());

	// 為每張牌創建 Widget
	for (int32 i = 0; i < Hand.Num(); ++i)
	{
		// 玩家 0 (自己) 顯示完整卡牌
		if (PlayerId == 0)
		{
			if (CardWidgetClass)
			{
				UCardWidget* NewCard = CreateWidget<UCardWidget>(this, CardWidgetClass);
				if (NewCard)
				{
					FCardData DisplayData;
					bool bDataFound = false;

					// 嘗試從 DataTable 獲取資料
					if (CardDataTable)
					{
						// 假設 RowName 就是 CardValue 的字串形式 (例如 "1", "2")
						FName RowName = FName(*FString::FromInt(Hand[i].CardValue));
						static const FString ContextString(TEXT("CardWidgetContext"));
						FCardData* CardData = CardDataTable->FindRow<FCardData>(RowName, ContextString);

						if (CardData)
						{
							DisplayData = *CardData;
							bDataFound = true;
							UE_LOG(LogTemp, Verbose, TEXT("HUD: Found data for card %d: %s"), Hand[i].CardValue, *DisplayData.Name);
						}
						else
						{
							UE_LOG(LogTemp, Warning, TEXT("HUD: Row '%s' not found in DataTable!"), *RowName.ToString());
						}
					}
					else
					{
						static bool bWarnedDT = false;
						if (!bWarnedDT)
						{
							UE_LOG(LogTemp, Error, TEXT("HUD: CardDataTable is NOT set in WBP_GameHUD!"));
							bWarnedDT = true;
						}
					}

					// 如果找不到資料，使用預設值
					if (!bDataFound)
					{
						DisplayData.Name = FString::Printf(TEXT("Card %d"), Hand[i].CardValue);
						DisplayData.Power = Hand[i].CardValue;
						DisplayData.Description = TEXT("No Data");
					}

					NewCard->UpdateCardDisplay(DisplayData);
					
					UHorizontalBoxSlot* HandSlot = Cast<UHorizontalBoxSlot>(HandBox->AddChild(NewCard));
					if (HandSlot)
					{
						// 設置間距，避免卡片擠在一起 (左右各50，總間距100)
						HandSlot->SetPadding(FMargin(50.0f, 0.0f, 50.0f, 0.0f));
						// 設置為自動大小，讓卡片保持自己的寬度
						HandSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
						// 垂直對齊填滿
						HandSlot->SetVerticalAlignment(VAlign_Fill);
					}
				}
			}
			else
			{
				// 只顯示一次警告
				static bool bWarned = false;
				if (!bWarned)
				{
					UE_LOG(LogTemp, Error, TEXT("CardWidgetClass not set in WBP_GameHUD! Please set it in the Widget Blueprint."));
					bWarned = true;
				}
			}
		}
		else
		{
			// 對手的手牌，顯示背面
			UTextBlock* CardText = NewObject<UTextBlock>(this);
			if (CardText)
			{
				CardText->SetText(FText::FromString(TEXT("[?]")));
				
				FSlateFontInfo FontInfo = CardText->GetFont();
				FontInfo.Size = 24;
				CardText->SetFont(FontInfo);

				UHorizontalBoxSlot* HandSlot = Cast<UHorizontalBoxSlot>(HandBox->AddChild(CardText));
				if (HandSlot)
				{
					HandSlot->SetPadding(FMargin(5.0f, 0.0f, 5.0f, 0.0f));
				}
			}
		}
	}
}

FString UCardGameHUD::GetBattleStateString(EBattleState State) const
{
	switch (State)
	{
	case EBattleState::Idle:
		return TEXT("Idle - Waiting to Start");
	case EBattleState::Started:
		return TEXT("Game Started");
	case EBattleState::WaitingForPlayer0:
		return TEXT("Waiting for Player 0");
	case EBattleState::WaitingForPlayer1:
		return TEXT("Waiting for Player 1");
	case EBattleState::RoundEnd:
		return TEXT("Round Ended");
	case EBattleState::GameOver:
		return TEXT("Game Over!");
	default:
		return TEXT("Unknown");
	}
}
