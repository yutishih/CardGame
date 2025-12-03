// CardTableManager.h
// 卡牌桌面管理器 - 管理 3D 卡牌的生成和佈局

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Card.h"
#include "CardTableManager.generated.h"

class ACard3DActor;
class ACardBattle;

UCLASS()
class CARDGAME_API ACardTableManager : public AActor
{
	GENERATED_BODY()

public:
	ACardTableManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 初始化並連接到 GameMode
	UFUNCTION(BlueprintCallable, Category = "CardTable")
	void Initialize(ACardBattle* InGameMode);

	// 更新玩家手牌顯示
	UFUNCTION(BlueprintCallable, Category = "CardTable")
	void UpdatePlayerHand(int32 PlayerId, const TArray<FCard>& HandCards);

	// 將卡牌移動到檯面（出牌）
	UFUNCTION(BlueprintCallable, Category = "CardTable")
	void PlayCardToTable(int32 PlayerId, int32 CardIndex, const FCard& Card);

	// 清空檯面上的牌
	UFUNCTION(BlueprintCallable, Category = "CardTable")
	void ClearTable();

	// 清空所有卡牌
	UFUNCTION(BlueprintCallable, Category = "CardTable")
	void ClearAllCards();

	// 處理卡牌點擊
	void OnCardClicked(ACard3DActor* ClickedCard);

protected:
	// 卡牌 Actor 類別（可在藍圖中覆蓋）
	UPROPERTY(EditDefaultsOnly, Category = "CardTable")
	TSubclassOf<ACard3DActor> CardActorClass;

	// 玩家手牌位置設定
	UPROPERTY(EditAnywhere, Category = "CardTable|Layout")
	FVector Player0HandCenter;

	UPROPERTY(EditAnywhere, Category = "CardTable|Layout")
	FVector Player1HandCenter;

	UPROPERTY(EditAnywhere, Category = "CardTable|Layout")
	float CardSpacing;

	UPROPERTY(EditAnywhere, Category = "CardTable|Layout")
	FVector Player0TablePosition;

	UPROPERTY(EditAnywhere, Category = "CardTable|Layout")
	FVector Player1TablePosition;

	// 卡牌在檯面上的間距
	UPROPERTY(EditAnywhere, Category = "CardTable|Layout")
	float TableCardSpacing;

private:
	// GameMode 引用
	UPROPERTY()
	TObjectPtr<ACardBattle> GameMode;

	// 玩家手牌 Actor 列表
	UPROPERTY()
	TArray<TObjectPtr<ACard3DActor>> Player0HandCards;

	UPROPERTY()
	TArray<TObjectPtr<ACard3DActor>> Player1HandCards;

	// 檯面上的卡牌
	UPROPERTY()
	TArray<TObjectPtr<ACard3DActor>> Player0TableCards;

	UPROPERTY()
	TArray<TObjectPtr<ACard3DActor>> Player1TableCards;

	// 生成卡牌 Actor
	ACard3DActor* SpawnCardActor(const FCard& Card, int32 PlayerId, FVector Location);

	// 計算手牌位置
	FVector CalculateHandCardPosition(int32 PlayerId, int32 CardIndex, int32 TotalCards);

	// 計算檯面卡牌位置
	FVector CalculateTableCardPosition(int32 PlayerId, int32 CardIndex);

	// 上一次更新的手牌數量
	int32 LastPlayer0HandCount;
	int32 LastPlayer1HandCount;
};
