// Copyright Epic Games, Inc. All Rights Reserved.

#include "Card.h"
#include "Data/DT_CardData.h"
#include "Engine/DataTable.h"

FCard::FCard()
	: CardValue(0)
{
}

FCard::FCard(int32 InValue)
	: CardValue(InValue)
{
}

bool FCard::IsValid() const
{
	return CardValue > 0 && CardValue <= 30;
}

UCardDeck::UCardDeck()
	: CurrentIndex(0)
{
}

void UCardDeck::Initialize()
{
	Deck.Empty();
	
	// 創建30張卡牌 (1-30)
	for (int32 i = 1; i <= 30; ++i)
	{
		Deck.Add(FCard(i));
	}

	ShuffleDeck();
	CurrentIndex = 0;
}

void UCardDeck::InitializeFromDataTable(UDataTable* DataTable)
{
	Deck.Empty();

	if (DataTable)
	{
		// 獲取所有 Row Names
		TArray<FName> RowNames = DataTable->GetRowNames();

		for (const FName& RowName : RowNames)
		{
			// 將 RowName 轉換為整數 ID
			FString RowString = RowName.ToString();
			if (RowString.IsNumeric())
			{
				int32 CardID = FCString::Atoi(*RowString);
				Deck.Add(FCard(CardID));
			}
		}
	}

	// 如果 DataTable 為空或讀取失敗，回退到預設邏輯
	if (Deck.Num() == 0)
	{
		Initialize();
		return;
	}

	ShuffleDeck();
	CurrentIndex = 0;
}

void UCardDeck::DrawCards(int32 NumberOfCards, TArray<FCard>& OutCards)
{
	OutCards.Empty();

	for (int32 i = 0; i < NumberOfCards && CurrentIndex < Deck.Num(); ++i)
	{
		OutCards.Add(Deck[CurrentIndex]);
		CurrentIndex++;
	}
}

void UCardDeck::Reset()
{
	Initialize();
}

void UCardDeck::ShuffleDeck()
{
	for (int32 i = Deck.Num() - 1; i > 0; --i)
	{
		int32 RandomIndex = FMath::RandRange(0, i);
		Deck.Swap(i, RandomIndex);
	}
}
