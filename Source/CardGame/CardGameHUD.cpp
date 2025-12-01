// Copyright Epic Games, Inc. All Rights Reserved.

#include "CardGameHUD.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
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

	// 為每張牌創建文字顯示
	for (int32 i = 0; i < Hand.Num(); ++i)
	{
		UTextBlock* CardText = NewObject<UTextBlock>(this);
		if (CardText)
		{
			// 玩家 0 顯示牌值，玩家 1 顯示背面（隱藏）
			if (PlayerId == 0)
			{
				CardText->SetText(FText::FromString(FString::Printf(TEXT("[%d]"), Hand[i].CardValue)));
			}
			else
			{
				CardText->SetText(FText::FromString(TEXT("[?]")));
			}

			FSlateFontInfo FontInfo = CardText->GetFont();
			FontInfo.Size = 24;
			CardText->SetFont(FontInfo);

			HandBox->AddChild(CardText);
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
