// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Card.generated.h"

/**
 * FCard - 代表一張卡牌
 * 每張卡牌有一個1-30的數值
 */
USTRUCT(BlueprintType)
struct FCard
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Card")
	int32 CardValue;

	FCard();
	FCard(int32 InValue);

	bool IsValid() const;
};

/**
 * 卡牌池 - 包含30張卡牌 (1-30)
 */
UCLASS()
class CARDGAME_API UCardDeck : public UObject
{
	GENERATED_BODY()

public:
	UCardDeck();

	// 初始化牌組
	void Initialize();

	// 從牌組中抽取指定數量的卡牌
	void DrawCards(int32 NumberOfCards, TArray<FCard>& OutCards);

	// 獲取剩餘的卡牌數量
	int32 GetRemainingCardsCount() const { return Deck.Num(); }

	// 重置牌組
	void Reset();

private:
	// 牌組中的所有卡牌
	UPROPERTY()
	TArray<FCard> Deck;

	// 當前位置
	int32 CurrentIndex;

	// 打亂牌組
	void ShuffleDeck();
};
