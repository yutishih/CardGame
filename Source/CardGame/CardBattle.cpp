// Copyright Epic Games, Inc. All Rights Reserved.

#include "CardBattle.h"
#include "CardGamePlayer.h"
#include "CardGameHUD.h"
#include "Data/DT_CardData.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/MovementComponent.h"

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
	DefaultPawnClass = ACardGamePlayer::StaticClass();
}

UClass* ACardBattle::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	return ACardGamePlayer::StaticClass();
}

void ACardBattle::BeginPlay()
{
	Super::BeginPlay();

	// 創建 HUD
	CreateHUD();
	
	// 直接開始遊戲（StartGame 會調用 InitializeGame）
	StartGame();
}

void ACardBattle::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (NewPlayer)
	{
		// Ensure pawn is ACardGamePlayer and setup camera
		APawn* PlayerPawn = NewPlayer->GetPawn();
		if (PlayerPawn)
		{
			// Try to find a camera component by iteration (works for ACardGamePlayer and others)
			UCameraComponent* Cam = PlayerPawn->FindComponentByClass<UCameraComponent>();
			if (Cam)
			{
				Cam->SetRelativeLocation(FVector(0.0f, 0.0f, 5000.0f));
				Cam->SetRelativeRotation(FRotator(-90.0f, -90.0f, 0.0f));
			}
			else
			{
				// Move actor if no camera component found (fallback)
				PlayerPawn->SetActorLocation(FVector(0.0f, 0.0f, 5000.0f));
				PlayerPawn->SetActorRotation(FRotator(-90.0f, -90.0f, 0.0f));
			}
			
			// Force view target
			NewPlayer->SetViewTarget(PlayerPawn);
			
			// Also disable control rotation on the controller just in case
			NewPlayer->SetControlRotation(FRotator(-90.0f, -90.0f, 0.0f));

			// Setup Mouse Cursor and Input Mode being TopDown
			NewPlayer->bShowMouseCursor = true;
			NewPlayer->bEnableClickEvents = true;
			NewPlayer->bEnableMouseOverEvents = true;

			// Set Input Mode to Game And UI, do not lock mouse
			FInputModeGameAndUI InputMode;
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			InputMode.SetHideCursorDuringCapture(false);
			NewPlayer->SetInputMode(InputMode);
		}
	}
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
		
		// 使用 DataTable 中的 Power 作為分數
		int32 ScoreToAdd = GetCardPower(PlayedCard.CardValue);
		Players[0]->AddScore(ScoreToAdd);
		
		Player0PlayedCards.Add(PlayedCard);  // 加入歷史記錄
		UE_LOG(LogTemp, Warning, TEXT("Player 0 played %d (Power: %d), score now: %d"), PlayedCard.CardValue, ScoreToAdd, Players[0]->GetScore());
	}
	else
	{
		CurrentRoundPlayer1Card = PlayedCard;
		bPlayer1CardPlayed = true;
		
		// 使用 DataTable 中的 Power 作為分數
		int32 ScoreToAdd = GetCardPower(PlayedCard.CardValue);
		Players[1]->AddScore(ScoreToAdd);
		
		Player1PlayedCards.Add(PlayedCard);  // 加入歷史記錄
		UE_LOG(LogTemp, Warning, TEXT("Player 1 played %d (Power: %d), score now: %d"), PlayedCard.CardValue, ScoreToAdd, Players[1]->GetScore());
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

		if (CardDataTable)
		{
			PlayerDecks[i]->InitializeFromDataTable(CardDataTable);
		}
		else
		{
			PlayerDecks[i]->Initialize();
		}

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
	// 切換讓另一位玩家開始新的一回合 (避免同一人連續出牌：結束上一局又開始下一局)
	CurrentTurnPlayerId = 1 - CurrentTurnPlayerId;

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
				
				int32 ScoreToAdd = GetCardPower(RandomCard.CardValue);
				Players[0]->AddScore(ScoreToAdd);
				Player0PlayedCards.Add(RandomCard);  // 加入歷史記錄
				UE_LOG(LogTemp, Warning, TEXT("Player 0 auto-played %d (Power: %d), score now: %d"), 
					RandomCard.CardValue, ScoreToAdd, Players[0]->GetScore());
			}
			else
			{
				CurrentRoundPlayer1Card = RandomCard;
				bPlayer1CardPlayed = true;
				
				int32 ScoreToAdd = GetCardPower(RandomCard.CardValue);
				Players[1]->AddScore(ScoreToAdd);
				Player1PlayedCards.Add(RandomCard);  // 加入歷史記錄
				UE_LOG(LogTemp, Warning, TEXT("Player 1 auto-played %d (Power: %d), score now: %d"), 
					RandomCard.CardValue, ScoreToAdd, Players[1]->GetScore());
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
		
		int32 ScoreToAdd = GetCardPower(AICard.CardValue);
		Players[1]->AddScore(ScoreToAdd);
		Player1PlayedCards.Add(AICard);  // 加入歷史記錄
		UE_LOG(LogTemp, Warning, TEXT("AI played %d (Power: %d), score now: %d"), AICard.CardValue, ScoreToAdd, Players[1]->GetScore());

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
	if (HUDWidgetClass)
	{
		GameHUD = CreateWidget<UCardGameHUD>(PC, HUDWidgetClass);
		if (GameHUD)
		{
			GameHUD->InitializeHUD(this);
			GameHUD->AddToViewport();
			UE_LOG(LogTemp, Warning, TEXT("Game HUD created successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to create Game HUD"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HUDWidgetClass not set in CardBattle GameMode"));
	}
}

int32 ACardBattle::GetCardPower(int32 CardValue) const
{
	if (!CardDataTable)
	{
		// 如果沒有設定 DataTable，預設回傳 CardValue
		return CardValue;
	}

	// 假設 RowName 就是 CardValue 的字串形式
	FName RowName = FName(*FString::FromInt(CardValue));
	static const FString ContextString(TEXT("GetCardPower"));
	FCardData* CardData = CardDataTable->FindRow<FCardData>(RowName, ContextString);

	if (CardData)
	{
		return CardData->Power;
	}

	// 找不到資料時回傳 0
	return 0;
}
