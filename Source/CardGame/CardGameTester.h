// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardBattle.h"
#include "CardGameTester.generated.h"

/**
 * ACardGameTester - 用於測試卡牌對戰系統的測試 Actor
 * 可以自動進行一局遊戲並輸出結果
 */
UCLASS()
class CARDGAME_API ACardGameTester : public AActor
{
	GENERATED_BODY()

public:
	ACardGameTester();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 開始測試遊戲
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void StartTestGame();

	// 停止測試遊戲
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void StopTestGame();

	// 進行一步自動模擬 (雙方自動隨機出牌直到遊戲結束)
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void SimulateGameRound();

	// 獲取測試狀態
	UFUNCTION(BlueprintCallable, Category = "Testing")
	bool IsTestingGame() const { return bIsTestingGame; }

	// 設置自動模擬的速度 (回合間延遲秒數)
	UFUNCTION(BlueprintCallable, Category = "Testing")
	void SetAutoPlayDelay(float Delay) { AutoPlayDelaySeconds = Delay; }

private:
	// 對戰遊戲模式指針
	UPROPERTY()
	ACardBattle* BattleGameMode;

	// 是否正在進行測試
	bool bIsTestingGame;

	// 自動出牌延遲
	float AutoPlayDelaySeconds;

	// 計時器
	float TimeSinceLastAutoPlay;

	// 輸出遊戲狀態到日誌
	void LogGameState();

	// 自動玩一局遊戲
	void AutoPlayRound();
};
