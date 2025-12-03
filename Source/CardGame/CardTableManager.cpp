// CardTableManager.cpp

#include "CardTableManager.h"
#include "Card3DActor.h"
#include "CardBattle.h"
#include "Kismet/GameplayStatics.h"

ACardTableManager::ACardTableManager()
{
	PrimaryActorTick.bCanEverTick = true;

	// 默認佈局設定（適合俯視或斜視角）
	// 玩家0手牌在下方（靠近螢幕底部/玩家視野）
	Player0HandCenter = FVector(200, 0, 100);
	// 玩家1手牌在上方（遠離玩家視野）
	Player1HandCenter = FVector(-200, 0, 100);
	// 卡牌間距
	CardSpacing = 100.0f;
	
	// 檯面位置（中間區域）
	Player0TablePosition = FVector(50, -200, 100);
	Player1TablePosition = FVector(-50, -200, 100);
	TableCardSpacing = 80.0f;

	LastPlayer0HandCount = 0;
	LastPlayer1HandCount = 0;
}

void ACardTableManager::BeginPlay()
{
	Super::BeginPlay();

	// 如果沒有設置 CardActorClass，使用默認的
	if (!CardActorClass)
	{
		CardActorClass = ACard3DActor::StaticClass();
	}

	// 自動尋找 GameMode
	if (!GameMode)
	{
		AGameModeBase* GM = UGameplayStatics::GetGameMode(GetWorld());
		GameMode = Cast<ACardBattle>(GM);
	}
}

void ACardTableManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 自動更新手牌（如果 GameMode 存在）
	if (GameMode)
	{
		const TArray<FCard>& Hand0 = GameMode->GetPlayerHand(0);
		const TArray<FCard>& Hand1 = GameMode->GetPlayerHand(1);

		if (Hand0.Num() != LastPlayer0HandCount)
		{
			UpdatePlayerHand(0, Hand0);
			LastPlayer0HandCount = Hand0.Num();
		}

		if (Hand1.Num() != LastPlayer1HandCount)
		{
			UpdatePlayerHand(1, Hand1);
			LastPlayer1HandCount = Hand1.Num();
		}
	}
}

void ACardTableManager::Initialize(ACardBattle* InGameMode)
{
	GameMode = InGameMode;
}

void ACardTableManager::UpdatePlayerHand(int32 PlayerId, const TArray<FCard>& HandCards)
{
	TArray<TObjectPtr<ACard3DActor>>& HandCardActors = (PlayerId == 0) ? Player0HandCards : Player1HandCards;

	// 如果卡牌數量減少，移除多餘的 Actor
	while (HandCardActors.Num() > HandCards.Num())
	{
		ACard3DActor* CardToRemove = HandCardActors.Pop();
		if (CardToRemove)
		{
			CardToRemove->Destroy();
		}
	}

	// 更新現有卡牌並添加新卡牌
	for (int32 i = 0; i < HandCards.Num(); ++i)
	{
		FVector TargetLocation = CalculateHandCardPosition(PlayerId, i, HandCards.Num());

		if (i < HandCardActors.Num())
		{
			// 更新現有卡牌
			if (HandCardActors[i])
			{
				HandCardActors[i]->SetCardData(HandCards[i]);
				HandCardActors[i]->SetHandIndex(i);
				HandCardActors[i]->SetOwnerPlayer(PlayerId);  // 確保擁有者正確
				HandCardActors[i]->MoveToLocation(TargetLocation, 0.3f);
				
				// 玩家自己的牌翻正面，對手的牌翻背面
				HandCardActors[i]->FlipCard(PlayerId == 0, false);
			}
		}
		else
		{
			// 生成新卡牌
			ACard3DActor* NewCard = SpawnCardActor(HandCards[i], PlayerId, TargetLocation);
			if (NewCard)
			{
				NewCard->SetHandIndex(i);
				NewCard->SetOwnerPlayer(PlayerId);  // 再次確保擁有者正確
				// 玩家自己的牌翻正面，對手的牌翻背面
				NewCard->FlipCard(PlayerId == 0, false);
				HandCardActors.Add(NewCard);
				UE_LOG(LogTemp, Warning, TEXT("Added card to Player %d hand, Index: %d, Value: %d"), PlayerId, i, HandCards[i].CardValue);
			}
		}
	}
}

void ACardTableManager::PlayCardToTable(int32 PlayerId, int32 CardIndex, const FCard& Card)
{
	TArray<TObjectPtr<ACard3DActor>>& HandCardActors = (PlayerId == 0) ? Player0HandCards : Player1HandCards;
	TArray<TObjectPtr<ACard3DActor>>& TableCardActors = (PlayerId == 0) ? Player0TableCards : Player1TableCards;

	ACard3DActor* CardActor = nullptr;

	// 從手牌中移除
	if (CardIndex >= 0 && CardIndex < HandCardActors.Num())
	{
		CardActor = HandCardActors[CardIndex];
		HandCardActors.RemoveAt(CardIndex);

		// 更新剩餘手牌的索引
		for (int32 i = CardIndex; i < HandCardActors.Num(); ++i)
		{
			if (HandCardActors[i])
			{
				HandCardActors[i]->SetHandIndex(i);
			}
		}
	}
	else
	{
		// 如果找不到對應的卡牌 Actor，創建一個新的
		FVector SpawnLocation = CalculateTableCardPosition(PlayerId, TableCardActors.Num());
		CardActor = SpawnCardActor(Card, PlayerId, SpawnLocation);
	}

	if (CardActor)
	{
		// 移動到檯面位置
		FVector TableLocation = CalculateTableCardPosition(PlayerId, TableCardActors.Num());
		CardActor->MoveToLocation(TableLocation, 0.5f);
		
		// 翻開卡牌
		CardActor->FlipCard(true, true);
		
		TableCardActors.Add(CardActor);
	}

	// 重新排列剩餘手牌
	for (int32 i = 0; i < HandCardActors.Num(); ++i)
	{
		if (HandCardActors[i])
		{
			FVector NewLocation = CalculateHandCardPosition(PlayerId, i, HandCardActors.Num());
			HandCardActors[i]->MoveToLocation(NewLocation, 0.3f);
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Card played to table: Player %d, Card Value %d"), PlayerId, Card.CardValue);
}

void ACardTableManager::ClearTable()
{
	for (ACard3DActor* Card : Player0TableCards)
	{
		if (Card)
		{
			Card->Destroy();
		}
	}
	Player0TableCards.Empty();

	for (ACard3DActor* Card : Player1TableCards)
	{
		if (Card)
		{
			Card->Destroy();
		}
	}
	Player1TableCards.Empty();
}

void ACardTableManager::ClearAllCards()
{
	ClearTable();

	for (ACard3DActor* Card : Player0HandCards)
	{
		if (Card)
		{
			Card->Destroy();
		}
	}
	Player0HandCards.Empty();

	for (ACard3DActor* Card : Player1HandCards)
	{
		if (Card)
		{
			Card->Destroy();
		}
	}
	Player1HandCards.Empty();

	LastPlayer0HandCount = 0;
	LastPlayer1HandCount = 0;
}

void ACardTableManager::OnCardClicked(ACard3DActor* ClickedCard)
{
	if (!ClickedCard || !GameMode)
	{
		return;
	}

	// 只有玩家 0 的手牌可以點擊
	int32 HandIndex = ClickedCard->GetHandIndex();
	if (HandIndex >= 0 && Player0HandCards.Contains(ClickedCard))
	{
		// 檢查是否是玩家的回合
		if (GameMode->GetBattleState() == EBattleState::WaitingForPlayer0)
		{
			UE_LOG(LogTemp, Warning, TEXT("3D Card clicked: index %d"), HandIndex);
			GameMode->PlayerPlayCard(0, HandIndex);
		}
	}
}

ACard3DActor* ACardTableManager::SpawnCardActor(const FCard& Card, int32 PlayerId, FVector Location)
{
	if (!CardActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CardActorClass is null!"));
		return nullptr;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACard3DActor* NewCard = GetWorld()->SpawnActor<ACard3DActor>(CardActorClass, Location, FRotator::ZeroRotator, SpawnParams);
	
	if (NewCard)
	{
		NewCard->SetOwnerPlayer(PlayerId);  // 先設置擁有者
		NewCard->SetCardData(Card);
		NewCard->SetCardTableManager(this);  // 設置管理器引用，用於點擊回調
		UE_LOG(LogTemp, Warning, TEXT("Spawned card for Player %d at %s, Value: %d"), PlayerId, *Location.ToString(), Card.CardValue);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to spawn card actor!"));
	}

	return NewCard;
}

FVector ACardTableManager::CalculateHandCardPosition(int32 PlayerId, int32 CardIndex, int32 TotalCards)
{
	FVector Center = (PlayerId == 0) ? Player0HandCenter : Player1HandCenter;
	
	// 計算卡牌排列的起始位置（居中）
	float TotalWidth = (TotalCards - 1) * CardSpacing;
	float StartX = -TotalWidth / 2.0f;
	
	FVector Position = Center;
	Position.Y = StartX + CardIndex * CardSpacing;
	
	// 添加弧形效果
	float NormalizedIndex = (TotalCards > 1) ? (float(CardIndex) / (TotalCards - 1)) * 2.0f - 1.0f : 0.0f;
	Position.Z += -FMath::Abs(NormalizedIndex) * 20.0f; // 邊緣的牌稍微低一點
	
	return Position;
}

FVector ACardTableManager::CalculateTableCardPosition(int32 PlayerId, int32 CardIndex)
{
	FVector BasePosition = (PlayerId == 0) ? Player0TablePosition : Player1TablePosition;
	
	// 橫向排列
	BasePosition.Y += CardIndex * TableCardSpacing;
	
	return BasePosition;
}
