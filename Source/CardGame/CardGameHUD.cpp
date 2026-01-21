// Copyright Epic Games, Inc. All Rights Reserved.

#include "CardGameHUD.h"
#include "UI/CardWidget.h"
#include "Data/DT_CardData.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBoxSlot.h"
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

	// 嘗試將手牌容器置中
	if (Player0HandBox)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Player0HandBox->Slot))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 1.0f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
			CanvasSlot->SetPosition(FVector2D(0.0f, -100.0f)); // 往上移動一點
			CanvasSlot->SetAutoSize(true);
		}
		else if (UVerticalBoxSlot* VSlot = Cast<UVerticalBoxSlot>(Player0HandBox->Slot))
		{
			VSlot->SetHorizontalAlignment(HAlign_Center);
		}
	}

	if (Player1HandBox)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Player1HandBox->Slot))
		{
			CanvasSlot->SetAnchors(FAnchors(0.5f, 0.0f));
			CanvasSlot->SetAlignment(FVector2D(0.5f, 0.0f));
			CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f)); // 確保位置偏移歸零
			CanvasSlot->SetAutoSize(true);
		}
		else if (UVerticalBoxSlot* VSlot = Cast<UVerticalBoxSlot>(Player1HandBox->Slot))
		{
			VSlot->SetHorizontalAlignment(HAlign_Center);
		}
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

	// 更新檯面的牌
	if (Player0CardBoard)
	{
		UpdatePlayedCards(0, Player0CardBoard);
	}
	
	if (Player1CardBoard)
	{
		UpdatePlayedCards(1, Player1CardBoard);
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

	// 獲取玩家手牌
	const TArray<FCard>& Hand = BattleGameMode->GetPlayerHand(PlayerId);

	// 動態計算間距與旋轉參數
	const float MaxHandWidth = 900.0f; // 手牌最大寬度限制 (可依據螢幕解析度調整)
	const float CardWidth = 150.0f;    // 假設卡牌顯示寬度
	const float BasePadding = -25.0f;  // 預設負邊距 (重疊量)

	float CurrentPadding = BasePadding;

	// 如果卡牌數量多，計算需要的壓縮邊距
	// 總寬度 ~= Num * (CardWidth + 2 * Padding)
	if (Hand.Num() > 0)
	{
		float CurrentTotalWidth = (float)Hand.Num() * (CardWidth + 2.0f * BasePadding);
		if (CurrentTotalWidth > MaxHandWidth)
		{
			// 計算新的 Padding 以符合最大寬度
			// Padding = ((MaxWidth / Num) - CardWidth) / 2
			CurrentPadding = ((MaxHandWidth / (float)Hand.Num()) - CardWidth) * 0.5f;
		}
	}

	// 根據數量調整旋轉角度步長，避免扇形太寬
	float AngleStep = 5.0f;
	if (Hand.Num() > 8)
	{
		// 限制最大展開角度範圍
		AngleStep = 40.0f / ((float)Hand.Num() * 0.5f);
	}

	// 檢查是否需要重建 (數量不同時才重建)
	// 注意：這裡簡單用數量判斷。如果遊戲中有交換手牌等機制，可能需要更嚴謹的檢查 (例如檢查 CardID)
	if (HandBox->GetChildrenCount() == Hand.Num())
	{
		// 數量相同，嘗試更新現有 Widget
		bool bUpdateSuccess = true;
		for (int32 i = 0; i < Hand.Num(); ++i)
		{
			UWidget* ChildWidget = HandBox->GetChildAt(i);
			
			// 所有玩家都使用 CardWidget
			if (UCardWidget* CardWidget = Cast<UCardWidget>(ChildWidget))
			{
				// 獲取資料並更新
				FCardData DisplayData;
				bool bDataFound = false;
				if (CardDataTable)
				{
					FName RowName = FName(*FString::FromInt(Hand[i].CardValue));
					static const FString ContextString(TEXT("CardWidgetContext"));
					FCardData* CardData = CardDataTable->FindRow<FCardData>(RowName, ContextString);
					if (CardData)
					{
						DisplayData = *CardData;
						bDataFound = true;
					}
				}
				
				if (!bDataFound)
				{
					DisplayData.Name = FString::Printf(TEXT("Card %d"), Hand[i].CardValue);
					DisplayData.Power = Hand[i].CardValue;
					DisplayData.Description = TEXT("No Data");
				}
				
				CardWidget->UpdateCardDisplay(DisplayData);
				
				// 確保索引正確 (因為手牌可能會變動)
				CardWidget->CardIndex = i;
				
				// 只有玩家 0 (自己) 才綁定點擊事件
				if (PlayerId == 0)
				{
					CardWidget->SetOnClicked(FOnCardClicked::CreateUObject(this, &UCardGameHUD::OnCardClicked));
				}
				else
				{
					// 清除綁定，避免誤觸
					CardWidget->SetOnClicked(FOnCardClicked()); 
				}

				// 更新佈局參數 (動態調整 Padding)
				if (UHorizontalBoxSlot* HSlot = Cast<UHorizontalBoxSlot>(CardWidget->Slot))
				{
					HSlot->SetPadding(FMargin(CurrentPadding, 0.0f, CurrentPadding, 0.0f));
				}

				// 更新扇形效果
				float CenterIndex = (Hand.Num() - 1) / 2.0f;
				float DistanceFromCenter = i - CenterIndex;
				float RotationAngle = DistanceFromCenter * AngleStep; 

				CardWidget->SetRenderTransformPivot(FVector2D(0.5f, 2.0f));
				CardWidget->SetRenderTransformAngle(RotationAngle);
			}
			else
			{
				// 類型不對，強制重建
				bUpdateSuccess = false;
				break;
			}
		}

		if (bUpdateSuccess)
		{
			return; // 更新成功，不需要重建
		}
	}

	// 清空現有內容 (只有在數量不對或類型不對時才執行)
	HandBox->ClearChildren();

	// 為每張牌創建 Widget
	for (int32 i = 0; i < Hand.Num(); ++i)
	{
		// 所有玩家都顯示完整卡牌
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
					}
					else
					{
						// UE_LOG(LogTemp, Warning, TEXT("HUD: Row '%s' not found in DataTable!"), *RowName.ToString());
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
				
				// 設定索引和點擊回調
				NewCard->CardIndex = i;
				
				// 只有玩家 0 綁定點擊
				if (PlayerId == 0)
				{
					NewCard->SetOnClicked(FOnCardClicked::CreateUObject(this, &UCardGameHUD::OnCardClicked));
				}

				// 扇形效果計算
				float CenterIndex = (Hand.Num() - 1) / 2.0f;
				float DistanceFromCenter = i - CenterIndex;
				float RotationAngle = DistanceFromCenter * AngleStep; // 使用動態計算的 AngleStep

				// 設定旋轉軸心在卡片下方，產生扇形效果
				// 0.5 = X軸中心, 2.0 = Y軸 (卡片高度的 2 倍處，即卡片底部再往下一個卡片高度)
				NewCard->SetRenderTransformPivot(FVector2D(0.5f, 2.0f));
				NewCard->SetRenderTransformAngle(RotationAngle);

				// 縮小卡牌以防止超出螢幕
				NewCard->SetRenderScale(FVector2D(1.0f, 1.0f));

				UHorizontalBoxSlot* HandSlot = Cast<UHorizontalBoxSlot>(HandBox->AddChild(NewCard));
				if (HandSlot)
				{
					// 設置動態邊距
					HandSlot->SetPadding(FMargin(CurrentPadding, 0.0f, CurrentPadding, 0.0f));
					// 設置為自動大小，讓卡片保持自己的寬度
					HandSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
					// 垂直對齊改為底部對齊，避免被拉伸到整個容器高度 (500)
					HandSlot->SetVerticalAlignment(VAlign_Bottom);
				}
			}
		}
	}
}

void UCardGameHUD::UpdatePlayedCards(int32 PlayerId, UHorizontalBox* BoardBox)
{
	if (!BoardBox || !BattleGameMode)
	{
		return;
	}

	// 獲取玩家已出的牌
	// 根據 PlayerId 選擇對應的函數
	const TArray<FCard>& PlayedCards = (PlayerId == 0) ? 
		BattleGameMode->GetPlayer0PlayedCards() : 
		BattleGameMode->GetPlayer1PlayedCards();

	// 簡單檢查數量，若不同則重建
	if (BoardBox->GetChildrenCount() == PlayedCards.Num())
	{
		return;
	}

	BoardBox->ClearChildren();

	for (int32 i = 0; i < PlayedCards.Num(); ++i)
	{
		if (CardWidgetClass)
		{
			UCardWidget* NewCard = CreateWidget<UCardWidget>(this, CardWidgetClass);
			if (NewCard)
			{
				FCardData DisplayData;
				bool bDataFound = false;

				if (CardDataTable)
				{
					FName RowName = FName(*FString::FromInt(PlayedCards[i].CardValue));
					static const FString ContextString(TEXT("PlayedCardContext"));
					FCardData* CardData = CardDataTable->FindRow<FCardData>(RowName, ContextString);

					if (CardData)
					{
						DisplayData = *CardData;
						bDataFound = true;
					}
				}

				if (!bDataFound)
				{
					DisplayData.Name = FString::Printf(TEXT("Card %d"), PlayedCards[i].CardValue);
					DisplayData.Power = PlayedCards[i].CardValue;
					DisplayData.Description = TEXT("");
				}

				NewCard->UpdateCardDisplay(DisplayData);
				
				// 檯面上的牌不可點擊
				NewCard->SetIsEnabled(true); // 保持啟用才能看到，但移除點擊回調
				NewCard->SetOnClicked(FOnCardClicked()); 

				// 確保沒有旋轉或縮放
				NewCard->SetRenderTransformAngle(0.0f);
				NewCard->SetRenderScale(FVector2D(0.8f, 0.8f)); // 稍微縮小一點以適應版面

				UHorizontalBoxSlot* CardSlot = Cast<UHorizontalBoxSlot>(BoardBox->AddChild(NewCard));
				if (CardSlot)
				{
					// 排列整齊，不能重疊 -> 使用正數 Padding
					CardSlot->SetPadding(FMargin(5.0f));
					CardSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
					CardSlot->SetVerticalAlignment(VAlign_Center);
					CardSlot->SetHorizontalAlignment(HAlign_Center);
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
