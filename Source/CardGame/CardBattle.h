// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Card.h"
#include "BattlePlayer.h"
#include "CardBattle.generated.h"

// 遊戲狀態枚舉
UENUM(BlueprintType)
enum class EBattleState : uint8
{
	Idle = 0,			// 空閒
	Started = 1,		// 遊戲已開始
	WaitingForPlayer0 = 2,	// 等待玩家0出牌
	WaitingForPlayer1 = 3,	// 等待玩家1出牌
	RoundEnd = 4,		// 回合結束
	GameOver = 5		// 遊戲結束
};

// 回合信息
USTRUCT(BlueprintType)
struct FRoundInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FCard Player0Card;

	UPROPERTY(BlueprintReadWrite)
	FCard Player1Card;

	UPROPERTY(BlueprintReadWrite)
	int32 WinnerID; // 0, 1 或 -1 (平手)

	FRoundInfo()
		: Player0Card(FCard(0))
		, Player1Card(FCard(0))
		, WinnerID(-1)
	{
	}
};

/**
 * ACardBattle - 卡牌對戰的核心遊戲模式
 */
UCLASS()
class CARDGAME_API ACardBattle : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACardBattle();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 開始遊戲
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void StartGame();

	// 結束遊戲
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void EndGame();

	// 玩家出牌 (由玩家輸入調用)
	UFUNCTION(BlueprintCallable, Category = "Battle")
	void PlayerPlayCard(int32 PlayerId, int32 CardIndex);

	// 獲取當前遊戲狀態
	UFUNCTION(BlueprintCallable, Category = "Battle")
	EBattleState GetBattleState() const { return CurrentState; }

	// 獲取當前回合的玩家ID
	UFUNCTION(BlueprintCallable, Category = "Battle")
	int32 GetCurrentTurnPlayerId() const { return CurrentTurnPlayerId; }

	// 獲取玩家的手牌
	UFUNCTION(BlueprintCallable, Category = "Battle")
	const TArray<FCard>& GetPlayerHand(int32 PlayerId) const;

	// 獲取玩家的分數
	UFUNCTION(BlueprintCallable, Category = "Battle")
	int32 GetPlayerScore(int32 PlayerId) const;

	// 獲取剩餘時間
	UFUNCTION(BlueprintCallable, Category = "Battle")
	float GetRemainingTurnTime() const;

	// 獲取上一回合的信息
	UFUNCTION(BlueprintCallable, Category = "Battle")
	const FRoundInfo& GetLastRoundInfo() const { return LastRoundInfo; }

	// 獲取遊戲獲勝者 (只在遊戲結束時有效)
	UFUNCTION(BlueprintCallable, Category = "Battle")
	int32 GetWinner() const { return Winner; }

private:
	// 初始化遊戲
	void InitializeGame();

	// 重置遊戲
	void ResetGame();

	// 隨機決定先手玩家
	void DetermineFirstPlayer();

	// 進入下一個回合
	void GoToNextTurn();

	// 處理當前回合時間
	void HandleTurnTimer(float DeltaTime);

	// 根據卡牌數值比較，決定本回合的勝者並計算分數
	void ResolveRound(FCard Card0, FCard Card1);

	// 結算遊戲 - 檢查遊戲是否結束
	bool CheckGameOver();

	// 確定最終獲勝者
	void DetermineWinner();

	// 玩家列表
	UPROPERTY()
	UBattlePlayer* Players[2];

	// 玩家牌組
	UPROPERTY()
	UCardDeck* PlayerDecks[2];

	// 當前遊戲狀態
	UPROPERTY(BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	EBattleState CurrentState;

	// 當前回合的玩家ID
	UPROPERTY(BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	int32 CurrentTurnPlayerId;

	// 當前回合剩餘時間 (秒)
	UPROPERTY(BlueprintReadOnly, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	float CurrentTurnRemainingTime;

	// 每回合的時間限制 (秒)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle", meta = (AllowPrivateAccess = "true"))
	float TurnTimeLimit;

	// 上一回合的結果
	FRoundInfo LastRoundInfo;

	// 最終獲勝者 (-1 表示平手或遊戲未結束)
	int32 Winner;

	// 是否已出牌標誌
	bool bPlayer0CardPlayed;
	bool bPlayer1CardPlayed;

	// 本回合出牌
	FCard CurrentRoundPlayer0Card;
	FCard CurrentRoundPlayer1Card;
};
