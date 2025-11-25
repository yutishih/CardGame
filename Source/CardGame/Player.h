// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Card.h"
#include "Player.generated.h"

/**
 * APlayer - 代表遊戲中的一個玩家
 */
UCLASS()
class CARDGAME_API APlayer : public UObject
{
	GENERATED_BODY()

public:
	APlayer();

	// 初始化玩家
	void Initialize(int32 PlayerId);

	// 獲取玩家ID
	int32 GetPlayerId() const { return PlayerID; }

	// 設置玩家的牌組
	void SetDeck(UCardDeck* InDeck);

	// 獲取手牌
	const TArray<FCard>& GetHand() const { return Hand; }

	// 從牌組抽牌到手牌
	void DrawCardsToHand(int32 NumberOfCards);

	// 出牌 - 根據索引從手牌中移除並返回該卡牌
	FCard PlayCard(int32 CardIndex);

	// 隨機出牌 - 系統自動從手牌隨機選擇一張
	FCard PlayCardRandom();

	// 獲取手牌數量
	int32 GetHandSize() const { return Hand.Num(); }

	// 獲取累計分數
	int32 GetScore() const { return Score; }

	// 增加分數
	void AddScore(int32 Points) { Score += Points; }

	// 重置分數
	void ResetScore() { Score = 0; }

	// 檢查是否還有手牌
	bool HasCards() const { return Hand.Num() > 0; }

private:
	// 玩家ID (0 或 1)
	int32 PlayerID;

	// 玩家的牌組
	UPROPERTY()
	UCardDeck* Deck;

	// 玩家的手牌
	TArray<FCard> Hand;

	// 玩家的累計分數
	int32 Score;
};
