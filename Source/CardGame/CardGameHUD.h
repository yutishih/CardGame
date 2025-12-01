// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"
#include "CardBattle.h"
#include "CardGameHUD.generated.h"

/**
 * UCardGameHUD - 卡牌遊戲的主要 UI
 */
UCLASS()
class CARDGAME_API UCardGameHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// 初始化 HUD
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void InitializeHUD(ACardBattle* InBattleGameMode);

	// 更新 UI 顯示
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void UpdateUI();

	// 玩家點擊卡牌出牌
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void OnCardClicked(int32 CardIndex);

protected:
	// UI 組件綁定
	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> Player0ScoreText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> Player1ScoreText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> CurrentTurnText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> TimerText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UProgressBar> TimerProgressBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> GameStateText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> LastRoundResultText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UHorizontalBox> Player0HandBox;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UHorizontalBox> Player1HandBox;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly)
	TObjectPtr<UTextBlock> WinnerText;

	// 卡牌按鈕類（用於動態創建）
	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UUserWidget> CardButtonClass;

private:
	// 遊戲模式引用
	UPROPERTY()
	TObjectPtr<ACardBattle> BattleGameMode;

	// 更新玩家手牌顯示
	void UpdatePlayerHand(int32 PlayerId, UHorizontalBox* HandBox);

	// 獲取遊戲狀態文字
	FString GetBattleStateString(EBattleState State) const;
};
