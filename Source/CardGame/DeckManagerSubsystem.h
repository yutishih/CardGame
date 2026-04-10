#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DeckManagerSubsystem.generated.h"

/**
 * UDeckManagerSubsystem
 * GameInstance 子系統，跨場景持有並管理玩家自訂牌組
 * 在 Initialize() 時自動從硬碟讀取存檔
 */
UCLASS()
class CARDGAME_API UDeckManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ── 牌組查詢 ────────────────────────────────────────────────

	// 取得目前玩家牌組（DataTable RowName 陣列）
	UFUNCTION(BlueprintCallable, Category = "Deck")
	const TArray<FName>& GetPlayerDeck() const { return PlayerDeck; }

	// 取得牌組張數
	UFUNCTION(BlueprintCallable, Category = "Deck")
	int32 GetDeckSize() const { return PlayerDeck.Num(); }

	// 是否有已存的自訂牌組
	UFUNCTION(BlueprintCallable, Category = "Deck")
	bool HasCustomDeck() const { return PlayerDeck.Num() > 0; }

	// 取得某張牌在牌組中的數量
	UFUNCTION(BlueprintCallable, Category = "Deck")
	int32 GetCardCount(FName RowName) const;

	// ── 牌組編輯 ────────────────────────────────────────────────

	// 覆寫整個牌組
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void SetPlayerDeck(const TArray<FName>& NewDeck);

	// 新增一張牌（受 MaxDeckSize / MaxCopiesPerCard 限制）
	// 回傳 true 代表新增成功
	UFUNCTION(BlueprintCallable, Category = "Deck")
	bool AddCard(FName RowName);

	// 移除一張牌（移除最後一個相符的）
	// 回傳 true 代表移除成功
	UFUNCTION(BlueprintCallable, Category = "Deck")
	bool RemoveCard(FName RowName);

	// 清空牌組
	UFUNCTION(BlueprintCallable, Category = "Deck")
	void ClearDeck();

	// ── 存檔 / 讀檔 ─────────────────────────────────────────────

	UFUNCTION(BlueprintCallable, Category = "Deck")
	void SaveDeck();

	UFUNCTION(BlueprintCallable, Category = "Deck")
	void LoadDeck();

	// ── 規則常數 ────────────────────────────────────────────────

	// 牌組最大張數
	static constexpr int32 MaxDeckSize = 30;

	// 同一張牌最多幾張
	static constexpr int32 MaxCopiesPerCard = 2;

private:
	// 記憶體中的牌組
	TArray<FName> PlayerDeck;
};
