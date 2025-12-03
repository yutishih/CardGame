// Card3DActor.h
// 3D 卡牌 Actor

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Card.h"
#include "Card3DActor.generated.h"

UENUM(BlueprintType)
enum class ECardSide : uint8
{
	Front,	// 正面（顯示數值）
	Back	// 背面（隱藏）
};

UCLASS()
class CARDGAME_API ACard3DActor : public AActor
{
	GENERATED_BODY()

public:
	ACard3DActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// 游標懸停事件
	virtual void NotifyActorBeginCursorOver() override;
	virtual void NotifyActorEndCursorOver() override;

	// 點擊事件
	virtual void NotifyActorOnClicked(FKey ButtonPressed = EKeys::LeftMouseButton) override;

	// 設置卡牌數據
	UFUNCTION(BlueprintCallable, Category = "Card")
	void SetCardData(const FCard& InCard);

	// 設置卡牌擁有者（0 = 玩家, 1 = 對手）
	UFUNCTION(BlueprintCallable, Category = "Card")
	void SetOwnerPlayer(int32 PlayerId);

	// 翻轉卡牌
	UFUNCTION(BlueprintCallable, Category = "Card")
	void FlipCard(bool bShowFront, bool bAnimate = true);

	// 移動到目標位置
	UFUNCTION(BlueprintCallable, Category = "Card")
	void MoveToLocation(FVector TargetLocation, float Duration = 0.5f);

	// 設置高亮（滑鼠懸停）
	UFUNCTION(BlueprintCallable, Category = "Card")
	void SetHighlight(bool bHighlight);

	// 獲取卡牌數據
	UFUNCTION(BlueprintCallable, Category = "Card")
	FCard GetCardData() const { return CardData; }

	// 獲取手牌索引
	UFUNCTION(BlueprintCallable, Category = "Card")
	int32 GetHandIndex() const { return HandIndex; }

	// 設置手牌索引
	UFUNCTION(BlueprintCallable, Category = "Card")
	void SetHandIndex(int32 Index) { HandIndex = Index; }

	// 是否正在動畫中
	UFUNCTION(BlueprintCallable, Category = "Card")
	bool IsAnimating() const { return bIsAnimating; }

	// 設置卡牌管理器引用
	void SetCardTableManager(class ACardTableManager* Manager) { CardTableManager = Manager; }

protected:
	// 卡牌根組件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> RootSceneComponent;

	// 卡牌網格（正面）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CardMeshFront;

	// 卡牌網格（背面）
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> CardMeshBack;

	// 卡牌數值文字組件
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<class UTextRenderComponent> CardValueText;

	// 卡牌數據
	UPROPERTY(BlueprintReadOnly, Category = "Card")
	FCard CardData;

	// 擁有者玩家 ID
	UPROPERTY(BlueprintReadOnly, Category = "Card")
	int32 OwnerPlayerId;

	// 手牌中的索引
	UPROPERTY(BlueprintReadOnly, Category = "Card")
	int32 HandIndex;

	// 當前顯示的面
	UPROPERTY(BlueprintReadOnly, Category = "Card")
	ECardSide CurrentSide;

	// 動畫相關
	bool bIsAnimating;
	bool bIsFlipping;
	bool bIsMoving;
	
	// 翻轉動畫
	float FlipProgress;
	float FlipDuration;
	bool bFlipToFront;
	
	// 移動動畫
	FVector MoveStartLocation;
	FVector MoveTargetLocation;
	float MoveProgress;
	float MoveDuration;

	// 材質
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> FrontMaterial;
	
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> BackMaterial;

	// 更新動畫
	void UpdateFlipAnimation(float DeltaTime);
	void UpdateMoveAnimation(float DeltaTime);

	// 更新卡牌顯示
	void UpdateCardDisplay();

	// 創建默認材質
	void CreateDefaultMaterials();

	// 卡牌管理器引用
	UPROPERTY()
	TObjectPtr<class ACardTableManager> CardTableManager;

	// 懸停狀態
	bool bIsHovered;
	FVector OriginalLocation;
};
