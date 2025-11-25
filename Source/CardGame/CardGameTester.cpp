// Copyright Epic Games, Inc. All Rights Reserved.

#include "CardGameTester.h"

ACardGameTester::ACardGameTester()
	: bIsTestingGame(false)
	, AutoPlayDelaySeconds(1.0f)
	, TimeSinceLastAutoPlay(0.0f)
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACardGameTester::BeginPlay()
{
	Super::BeginPlay();

	// 獲取遊戲模式
	BattleGameMode = Cast<ACardBattle>(GetWorld()->GetAuthGameMode());

	if (!BattleGameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("CardGameTester: Could not find ACardBattle GameMode!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CardGameTester: Found ACardBattle GameMode"));
	}
}

void ACardGameTester::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsTestingGame && BattleGameMode)
	{
		TimeSinceLastAutoPlay += DeltaTime;

		if (TimeSinceLastAutoPlay >= AutoPlayDelaySeconds)
		{
			TimeSinceLastAutoPlay = 0.0f;
			AutoPlayRound();
		}
	}
}

void ACardGameTester::StartTestGame()
{
	if (!BattleGameMode)
	{
		UE_LOG(LogTemp, Error, TEXT("CardGameTester: No BattleGameMode available"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("===== STARTING TEST GAME ====="));

	bIsTestingGame = true;
	TimeSinceLastAutoPlay = 0.0f;

	BattleGameMode->StartGame();
	LogGameState();
}

void ACardGameTester::StopTestGame()
{
	UE_LOG(LogTemp, Warning, TEXT("===== STOPPING TEST GAME ====="));

	bIsTestingGame = false;
	TimeSinceLastAutoPlay = 0.0f;

	if (BattleGameMode)
	{
		BattleGameMode->EndGame();
	}
}

void ACardGameTester::SimulateGameRound()
{
	if (!BattleGameMode)
	{
		return;
	}

	if (BattleGameMode->GetBattleState() == EBattleState::GameOver)
	{
		LogGameState();
		StopTestGame();
		return;
	}

	AutoPlayRound();
}

void ACardGameTester::LogGameState()
{
	if (!BattleGameMode)
	{
		return;
	}

	EBattleState State = BattleGameMode->GetBattleState();
	int32 Player0Score = BattleGameMode->GetPlayerScore(0);
	int32 Player1Score = BattleGameMode->GetPlayerScore(1);
	const TArray<FCard>& Player0Hand = BattleGameMode->GetPlayerHand(0);
	const TArray<FCard>& Player1Hand = BattleGameMode->GetPlayerHand(1);

	UE_LOG(LogTemp, Warning, TEXT("========== GAME STATE =========="));
	UE_LOG(LogTemp, Warning, TEXT("Battle State: %d"), (int32)State);
	UE_LOG(LogTemp, Warning, TEXT("Player 0 - Hand: %d cards, Score: %d"), Player0Hand.Num(), Player0Score);
	UE_LOG(LogTemp, Warning, TEXT("Player 1 - Hand: %d cards, Score: %d"), Player1Hand.Num(), Player1Score);

	if (State == EBattleState::GameOver)
	{
		int32 Winner = BattleGameMode->GetWinner();
		if (Winner == 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("WINNER: Player 0"));
		}
		else if (Winner == 1)
		{
			UE_LOG(LogTemp, Warning, TEXT("WINNER: Player 1"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("RESULT: Draw"));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("================================"));
}

void ACardGameTester::AutoPlayRound()
{
	if (!BattleGameMode)
	{
		return;
	}

	EBattleState State = BattleGameMode->GetBattleState();

	if (State == EBattleState::GameOver)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game is over!"));
		bIsTestingGame = false;
		LogGameState();
		return;
	}

	if (State == EBattleState::WaitingForPlayer0 || State == EBattleState::WaitingForPlayer1)
	{
		int32 CurrentPlayer = BattleGameMode->GetCurrentTurnPlayerId();
		const TArray<FCard>& Hand = BattleGameMode->GetPlayerHand(CurrentPlayer);

		if (Hand.Num() > 0)
		{
			// 隨機選擇一張牌
			int32 RandomCardIndex = FMath::RandRange(0, Hand.Num() - 1);
			UE_LOG(LogTemp, Warning, TEXT("Player %d plays card (index %d, value %d)"),
				CurrentPlayer, RandomCardIndex, Hand[RandomCardIndex].CardValue);

			BattleGameMode->PlayerPlayCard(CurrentPlayer, RandomCardIndex);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Player %d has no cards left"), CurrentPlayer);
		}
	}
}
