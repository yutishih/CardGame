// Copyright Epic Games, Inc. All Rights Reserved.

#include "CardBattle.h"
#include "CardGameSimpleHUD.h"
#include "CardTableManager.h"
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
	
	// 創建 3D 卡牌系統
	Create3DCardSystem();
	
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

	// 記錄出牌並立刻加分
	if (PlayerId == 0)
	{
		CurrentRoundPlayer0Card = PlayedCard;
		bPlayer0CardPlayed = true;
		Players[0]->AddScore(PlayedCard.CardValue);
		Player0PlayedCards.Add(PlayedCard);  // 加入歷史記錄
		Notify3DCardPlayed(0, CardIndex, PlayedCard);  // 通知 3D 卡牌系統
		UE_LOG(LogTemp, Warning, TEXT("Player 0 played %d, score now: %d"), PlayedCard.CardValue, Players[0]->GetScore());
	}
	else
	{
		CurrentRoundPlayer1Card = PlayedCard;
		bPlayer1CardPlayed = true;
		Players[1]->AddScore(PlayedCard.CardValue);
		Player1PlayedCards.Add(PlayedCard);  // 加入歷史記錄
		Notify3DCardPlayed(1, CardIndex, PlayedCard);  // 通知 3D 卡牌系統
		UE_LOG(LogTemp, Warning, TEXT("Player 1 played %d, score now: %d"), PlayedCard.CardValue, Players[1]->GetScore());
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
		
		// 如果切換到 AI（Player 1）的回合，立刻自動出牌
		if (CurrentTurnPlayerId == 1)
		{
			AIPlayCard();
		}
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
	
	// 清空已出牌歷史
	Player0PlayedCards.Empty();
	Player1PlayedCards.Empty();
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
	
	// 如果是 AI（Player 1）的回合，立刻自動出牌
	if (CurrentTurnPlayerId == 1)
	{
		AIPlayCard();
	}
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
				Players[0]->AddScore(RandomCard.CardValue);
				Player0PlayedCards.Add(RandomCard);  // 加入歷史記錄
				Notify3DCardPlayed(0, 0, RandomCard);  // 通知 3D 卡牌系統（隨機出牌，索引用 0）
				UE_LOG(LogTemp, Warning, TEXT("Player 0 auto-played %d, score now: %d"), RandomCard.CardValue, Players[0]->GetScore());
			}
			else
			{
				CurrentRoundPlayer1Card = RandomCard;
				bPlayer1CardPlayed = true;
				Players[1]->AddScore(RandomCard.CardValue);
				Player1PlayedCards.Add(RandomCard);  // 加入歷史記錄
				Notify3DCardPlayed(1, 0, RandomCard);  // 通知 3D 卡牌系統
				UE_LOG(LogTemp, Warning, TEXT("Player 1 auto-played %d, score now: %d"), RandomCard.CardValue, Players[1]->GetScore());
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
	UE_LOG(LogTemp, Warning, TEXT("Round completed: Player0 played %d, Player1 played %d"), Card0.CardValue, Card1.CardValue);

	LastRoundInfo.Player0Card = Card0;
	LastRoundInfo.Player1Card = Card1;
	LastRoundInfo.WinnerID = -1;  // 不再判定回合勝負
	
	UE_LOG(LogTemp, Warning, TEXT("Current scores - Player 0: %d, Player 1: %d"), Players[0]->GetScore(), Players[1]->GetScore());
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

void ACardBattle::AIPlayCard()
{
	if (CurrentTurnPlayerId != 1 || !Players[1])
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("AI (Player 1) plays a card"));

	// AI 隨機出牌
	FCard AICard = Players[1]->PlayCardRandom();

	if (AICard.IsValid())
	{
		CurrentRoundPlayer1Card = AICard;
		bPlayer1CardPlayed = true;
		Players[1]->AddScore(AICard.CardValue);
		Player1PlayedCards.Add(AICard);  // 加入歷史記錄
		Notify3DCardPlayed(1, 0, AICard);  // 通知 3D 卡牌系統
		UE_LOG(LogTemp, Warning, TEXT("AI played %d, score now: %d"), AICard.CardValue, Players[1]->GetScore());

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
			// 切換到玩家 0 的回合
			CurrentTurnPlayerId = 0;
			CurrentState = EBattleState::WaitingForPlayer0;
			CurrentTurnRemainingTime = TurnTimeLimit;
		}
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

void ACardBattle::Create3DCardSystem()
{
	// 在場景中生成 CardTableManager
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	CardTableManager = GetWorld()->SpawnActor<ACardTableManager>(ACardTableManager::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	
	if (CardTableManager)
	{
		CardTableManager->Initialize(this);
		UE_LOG(LogTemp, Warning, TEXT("3D Card System created successfully"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create 3D Card System"));
	}
}

void ACardBattle::Notify3DCardPlayed(int32 PlayerId, int32 CardIndex, const FCard& Card)
{
	if (CardTableManager)
	{
		CardTableManager->PlayCardToTable(PlayerId, CardIndex, Card);
	}
}
