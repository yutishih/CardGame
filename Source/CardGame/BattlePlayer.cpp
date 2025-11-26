// Copyright Epic Games, Inc. All Rights Reserved.

#include "BattlePlayer.h"

UBattlePlayer::UBattlePlayer()
	: PlayerID(0)
	, Deck(nullptr)
	, Score(0)
{
}

void UBattlePlayer::Initialize(int32 PlayerId)
{
	PlayerID = PlayerId;
	Hand.Empty();
	Score = 0;
}

void UBattlePlayer::SetDeck(UCardDeck* InDeck)
{
	Deck = InDeck;
}

void UBattlePlayer::DrawCardsToHand(int32 NumberOfCards)
{
	if (!Deck)
	{
		return;
	}

	TArray<FCard> DrawnCards;
	Deck->DrawCards(NumberOfCards, DrawnCards);
	Hand.Append(DrawnCards);
}

FCard UBattlePlayer::PlayCard(int32 CardIndex)
{
	if (CardIndex >= 0 && CardIndex < Hand.Num())
	{
		FCard PlayedCard = Hand[CardIndex];
		Hand.RemoveAt(CardIndex);
		return PlayedCard;
	}

	return FCard(0);
}

FCard UBattlePlayer::PlayCardRandom()
{
	if (Hand.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, Hand.Num() - 1);
		return PlayCard(RandomIndex);
	}

	return FCard(0);
}
