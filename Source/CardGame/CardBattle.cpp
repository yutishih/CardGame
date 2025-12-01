// Copyright Epic Games, Inc. All Rights Reserved.

#include "CardBattle.h"
#include "CardGameSimpleHUD.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ACardBattle::ACardBattle()
	: CurrentState(EBattleState::Idle)
	, CurrentTurnPlayerId(0)
	, CurrentTurnRemainingTime(0.0f)
	, TurnTimeLimit(5.0f)  // 5 秒回合時間
	, Winner(-1)
	, bPlayer0CardPlayed(false)
	, bPlayer1CardPlayed(false)
	, CurrentRoundPlayer0Card(FCard(0))
	, CurrentRoundPlayer1Card(FCard(0))
{
	PrimaryActorTick.bCanEverTick = true;
}

void ACardBattle::BeginPlay()
{
	Super::BeginPlay();
	
	// 創建 HUD
	CreateHUD();
	
	// 直接開始遊戲（StartGame 會調用 InitializeGame）
	StartGame();
}

void ACardBattle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentState == EBattleState::WaitingForPlayer0 || CurrentState == EBattleState::WaitingForPlayer1)
	{
		HandleTurnTimer(DeltaTime);
	}
}

void ACardBattle::StartGame()
{
	if (CurrentState != EBattleState::Idle)
	{
		UE_LOG(LogTemp, Warning, TEXT("Game is already started or in progress"));
		return;
	}

	ResetGame();
	InitializeGame();

	// 隨機決定先手
	DetermineFirstPlayer();

	CurrentState = EBattleState::Started;
	GoToNextTurn();
}

void ACardBattle::EndGame()
{
	CurrentState = EBattleState::Idle;
	ResetGame();
}

void ACardBattle::PlayerPlayCard(int32 PlayerId, int32 CardIndex)
{
	// 檢查是否是當前玩家的回合
	if (CurrentState != EBattleState::WaitingForPlayer0 && CurrentState != EBattleState::WaitingForPlayer1)
	{
		return;
	}

	if (CurrentTurnPlayerId != PlayerId)
	{
		UE_LOG(LogTemp, Warning, TEXT("Not player %d's turn"), PlayerId);
		return;
	}

	if (!Players[PlayerId])
	{
		return;
	}

	// 玩家出牌
	FCard PlayedCard = Players[PlayerId]->PlayCard(CardIndex);

	if (!PlayedCard.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid card played by player %d"), PlayerId);
		return;
	}

	// 記錄出牌
	if (PlayerId == 0)
	{
		CurrentRoundPlayer0Card = PlayedCard;
		bPlayer0CardPlayed = true;
	}
	else
	{
		CurrentRoundPlayer1Card = PlayedCard;
		bPlayer1CardPlayed = true;
	}

	// 如果雙方都出牌了，結算本回合
	if (bPlayer0CardPlayed && bPlayer1CardPlayed)
	{
		ResolveRound(CurrentRoundPlayer0Card, CurrentRoundPlayer1Card);

		// 檢查遊戲是否結束
		if (!CheckGameOver())
		{
			// 進入下一回合
			GoToNextTurn();
		}
		else
		{
			DetermineWinner();
			CurrentState = EBattleState::GameOver;
		}
	}
	else
	{
		// 切換到另一玩家的回合
		CurrentTurnPlayerId = 1 - CurrentTurnPlayerId;
		CurrentState = CurrentTurnPlayerId == 0 ? EBattleState::WaitingForPlayer0 : EBattleState::WaitingForPlayer1;
		CurrentTurnRemainingTime = TurnTimeLimit;
	}
}

const TArray<FCard>& ACardBattle::GetPlayerHand(int32 PlayerId) const
{
	static TArray<FCard> EmptyHand;
	if (PlayerId >= 0 && PlayerId < 2 && Players[PlayerId])
	{
		return Players[PlayerId]->GetHand();
	}
	return EmptyHand;
}

int32 ACardBattle::GetPlayerScore(int32 PlayerId) const
{
	if (PlayerId >= 0 && PlayerId < 2 && Players[PlayerId])
	{
		return Players[PlayerId]->GetScore();
	}
	return 0;
}

float ACardBattle::GetRemainingTurnTime() const
{
	return FMath::Max(0.0f, CurrentTurnRemainingTime);
}

void ACardBattle::InitializeGame()
{
	// 創建玩家和牌組
	for (int32 i = 0; i < 2; ++i)
	{
		if (!Players[i])
		{
			Players[i] = NewObject<UBattlePlayer>();
			Players[i]->Initialize(i);
		}

		if (!PlayerDecks[i])
		{
			PlayerDecks[i] = NewObject<UCardDeck>();
		}

		PlayerDecks[i]->Initialize();
		Players[i]->SetDeck(PlayerDecks[i]);

		// 每個玩家抽10張牌
		Players[i]->DrawCardsToHand(10);
	}

	CurrentState = EBattleState::Idle;
	CurrentTurnPlayerId = 0;
	CurrentTurnRemainingTime = TurnTimeLimit;
	Winner = -1;
	bPlayer0CardPlayed = false;
	bPlayer1CardPlayed = false;

	UE_LOG(LogTemp, Warning, TEXT("Game initialized. Player 0 hand size: %d, Player 1 hand size: %d"),
		Players[0]->GetHandSize(), Players[1]->GetHandSize());
}

void ACardBattle::ResetGame()
{
	for (int32 i = 0; i < 2; ++i)
	{
		if (Players[i])
		{
			Players[i]->ResetScore();
		}
	}

	CurrentState = EBattleState::Idle;
	CurrentTurnPlayerId = 0;
	CurrentTurnRemainingTime = TurnTimeLimit;
	Winner = -1;
	bPlayer0CardPlayed = false;
	bPlayer1CardPlayed = false;
	CurrentRoundPlayer0Card = FCard(0);
	CurrentRoundPlayer1Card = FCard(0);
}

void ACardBattle::DetermineFirstPlayer()
{
	CurrentTurnPlayerId = FMath::RandRange(0, 1);
	UE_LOG(LogTemp, Warning, TEXT("Player %d goes first"), CurrentTurnPlayerId);
}

void ACardBattle::GoToNextTurn()
{
	// 重置本回合的出牌狀態
	bPlayer0CardPlayed = false;
	bPlayer1CardPlayed = false;
	CurrentRoundPlayer0Card = FCard(0);
	CurrentRoundPlayer1Card = FCard(0);
	CurrentTurnRemainingTime = TurnTimeLimit;

	CurrentState = CurrentTurnPlayerId == 0 ? EBattleState::WaitingForPlayer0 : EBattleState::WaitingForPlayer1;
}

void ACardBattle::HandleTurnTimer(float DeltaTime)
{
	CurrentTurnRemainingTime -= DeltaTime;

	if (CurrentTurnRemainingTime <= 0.0f)
	{
		CurrentTurnRemainingTime = 0.0f;

		// 系統隨機出牌
		UE_LOG(LogTemp, Warning, TEXT("Player %d time's up, system plays random card"), CurrentTurnPlayerId);

		FCard RandomCard = Players[CurrentTurnPlayerId]->PlayCardRandom();

		if (RandomCard.IsValid())
		{
			if (CurrentTurnPlayerId == 0)
			{
				CurrentRoundPlayer0Card = RandomCard;
				bPlayer0CardPlayed = true;
			}
			else
			{
				CurrentRoundPlayer1Card = RandomCard;
				bPlayer1CardPlayed = true;
			}

			// 如果雙方都出牌了，結算本回合
			if (bPlayer0CardPlayed && bPlayer1CardPlayed)
			{
				ResolveRound(CurrentRoundPlayer0Card, CurrentRoundPlayer1Card);

				// 檢查遊戲是否結束
				if (!CheckGameOver())
				{
					// 進入下一回合
					GoToNextTurn();
				}
				else
				{
					DetermineWinner();
					CurrentState = EBattleState::GameOver;
				}
			}
			else
			{
				// 切換到另一玩家的回合
				CurrentTurnPlayerId = 1 - CurrentTurnPlayerId;
				CurrentState = CurrentTurnPlayerId == 0 ? EBattleState::WaitingForPlayer0 : EBattleState::WaitingForPlayer1;
				CurrentTurnRemainingTime = TurnTimeLimit;
			}
		}
	}
}

void ACardBattle::ResolveRound(FCard Card0, FCard Card1)
{
	UE_LOG(LogTemp, Warning, TEXT("Round resolved: Player0 played %d, Player1 played %d"), Card0.CardValue, Card1.CardValue);

	LastRoundInfo.Player0Card = Card0;
	LastRoundInfo.Player1Card = Card1;

	if (Card0.CardValue > Card1.CardValue)
	{
		LastRoundInfo.WinnerID = 0;
		Players[0]->AddScore(Card0.CardValue + Card1.CardValue);
		UE_LOG(LogTemp, Warning, TEXT("Player 0 wins round, score: %d"), Card0.CardValue + Card1.CardValue);
	}
	else if (Card1.CardValue > Card0.CardValue)
	{
		LastRoundInfo.WinnerID = 1;
		Players[1]->AddScore(Card0.CardValue + Card1.CardValue);
		UE_LOG(LogTemp, Warning, TEXT("Player 1 wins round, score: %d"), Card0.CardValue + Card1.CardValue);
	}
	else
	{
		LastRoundInfo.WinnerID = -1;
		UE_LOG(LogTemp, Warning, TEXT("Round is a draw"));
	}
}

bool ACardBattle::CheckGameOver()
{
	// 如果雙方都沒有手牌，遊戲結束
	if (!Players[0]->HasCards() && !Players[1]->HasCards())
	{
		UE_LOG(LogTemp, Warning, TEXT("Both players out of cards, game over"));
		return true;
	}

	return false;
}

void ACardBattle::DetermineWinner()
{
	int32 Player0Score = Players[0]->GetScore();
	int32 Player1Score = Players[1]->GetScore();

	UE_LOG(LogTemp, Warning, TEXT("Final scores - Player 0: %d, Player 1: %d"), Player0Score, Player1Score);

	if (Player0Score > Player1Score)
	{
		Winner = 0;
		UE_LOG(LogTemp, Warning, TEXT("Player 0 wins the game!"));
	}
	else if (Player1Score > Player0Score)
	{
		Winner = 1;
		UE_LOG(LogTemp, Warning, TEXT("Player 1 wins the game!"));
	}
	else
	{
		Winner = -1;
		UE_LOG(LogTemp, Warning, TEXT("Game is a draw!"));
	}
}

void ACardBattle::CreateHUD()
{
	// 獲取第一個玩家控制器
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("No PlayerController found for HUD"));
		return;
	}

	// 啟用滑鼠游標和 UI 點擊
	PC->bShowMouseCursor = true;
	PC->bEnableClickEvents = true;
	PC->bEnableMouseOverEvents = true;
	
	// 設置輸入模式為 UI 和遊戲都可以控制
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PC->SetInputMode(InputMode);

	// 創建 HUD Widget
	GameHUD = CreateWidget<UCardGameSimpleHUD>(PC, UCardGameSimpleHUD::StaticClass());
	if (GameHUD)
	{
		GameHUD->SetBattleGameMode(this);
		GameHUD->AddToViewport();
		UE_LOG(LogTemp, Warning, TEXT("Game HUD created successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create Game HUD"));
	}
}
