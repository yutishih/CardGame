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
	
	// 玩家 0 手牌容器（用於動態生成按鈕）
	TSharedPtr<SHorizontalBox> Player0HandBox;

	void UpdateDisplay();
	void UpdatePlayerHandButtons();
	
	// 點擊卡牌出牌
	FReply OnCardClicked(int32 CardIndex);
};
