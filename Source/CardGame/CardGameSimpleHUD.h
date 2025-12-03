// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "CardBattle.h"
#include "CardGameSimpleHUD.generated.h"

/**
 * UCardGameSimpleHUD - 簡單的卡牌遊戲 HUD（純代碼生成）
 */
UCLASS()
class CARDGAME_API UCardGameSimpleHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 設置 GameMode
	void SetBattleGameMode(ACardBattle* InBattleGameMode);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	UPROPERTY()
	TObjectPtr<ACardBattle> BattleGameMode;

	// Slate 文字元件
	TSharedPtr<STextBlock> Player0ScoreText;
	TSharedPtr<STextBlock> Player1ScoreText;
	TSharedPtr<STextBlock> CurrentTurnText;
	TSharedPtr<STextBlock> TimerText;
	TSharedPtr<STextBlock> GameStateText;
	TSharedPtr<STextBlock> LastRoundText;
	TSharedPtr<STextBlock> WinnerText;
	TSharedPtr<STextBlock> Player1HandText;
	
	// 遊戲結束選單
	TSharedPtr<SVerticalBox> GameOverMenuBox;
	
	// 檯面上的牌（動態容器）
	TSharedPtr<SHorizontalBox> Player0PlayedCardsBox;
	TSharedPtr<SHorizontalBox> Player1PlayedCardsBox;
	
	// 玩家 0 手牌容器（用於動態生成按鈕）
	TSharedPtr<SHorizontalBox> Player0HandBox;

	void UpdateDisplay();
	void UpdatePlayerHandButtons();
	void UpdatePlayedCards();  // 更新檯面上的牌
	void UpdateGameOverMenu(); // 更新遊戲結束選單
	
	// 記錄已顯示的牌數量（用於檢測是否需要更新）
	int32 LastPlayer0CardCount = 0;
	int32 LastPlayer1CardCount = 0;
	
	// 點擊卡牌出牌
	FReply OnCardClicked(int32 CardIndex);
	
	// 遊戲結束選單按鈕回調
	FReply OnPlayAgainClicked();
	FReply OnBackToMenuClicked();
};
