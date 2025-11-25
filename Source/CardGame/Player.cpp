// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player.h"

APlayer::APlayer()
	: PlayerID(0)
	, Deck(nullptr)
	, Score(0)
{
}

void APlayer::Initialize(int32 PlayerId)
{
	PlayerID = PlayerId;
	Hand.Empty();
	Score = 0;
}

void APlayer::SetDeck(UCardDeck* InDeck)
{
	Deck = InDeck;
}

void APlayer::DrawCardsToHand(int32 NumberOfCards)
{
	if (!Deck)
	{
		return;
	}

	TArray<FCard> DrawnCards;
	Deck->DrawCards(NumberOfCards, DrawnCards);
	Hand.Append(DrawnCards);
}

FCard APlayer::PlayCard(int32 CardIndex)
{
	if (CardIndex >= 0 && CardIndex < Hand.Num())
	{
		FCard PlayedCard = Hand[CardIndex];
		Hand.RemoveAt(CardIndex);
		return PlayedCard;
	}

	return FCard(0);
}

FCard APlayer::PlayCardRandom()
{
	if (Hand.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, Hand.Num() - 1);
		return PlayCard(RandomIndex);
	}

	return FCard(0);
}

void APlayer::AddScore(int32 Points)
{
	Score += Points;
}

void APlayer::ResetScore()
{
	Score = 0;
}
