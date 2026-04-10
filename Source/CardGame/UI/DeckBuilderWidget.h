#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class UDataTable;
class UDeckManagerSubsystem;

/**
 * SDeckBuilderWidget - Slate 牌組編輯器
 *
 * 佈局：
 *   ┌──────────────────────────────────────────────┐
 *   │  DECK BUILDER           [存檔] [返回]        │
 *   ├──────────────────┬───────────────────────────┤
 *   │  所有卡牌 (左)   │  我的牌組 X/30 (右)      │
 *   │  [+] 卡名  Power │  [-] 卡名  Power  x張    │
 *   │  ...             │  ...                      │
 *   └──────────────────┴───────────────────────────┘
 *
 * 使用方式：
 *   1. 呼叫 Initialize(DataTable, DeckManager)
 *   2. 設定 OnCloseRequested 委派（由 GameMode 監聽以隱藏此 Widget）
 */
class CARDGAME_API SDeckBuilderWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDeckBuilderWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// 傳入資料來源與牌組管理器
	void Initialize(UDataTable* InDataTable, UDeckManagerSubsystem* InDeckManager);

	// 當使用者按下「返回」時觸發
	FSimpleDelegate OnCloseRequested;

private:
	// ── 資料 ────────────────────────────────────────────────────

	TWeakObjectPtr<UDataTable>           CardDataTable;
	TWeakObjectPtr<UDeckManagerSubsystem> DeckManager;

	// 目前編輯中的牌組（在儲存前不寫入 Subsystem）
	TArray<FName> EditingDeck;

	// 所有可用卡牌的 RowName 清單
	TArray<TSharedPtr<FName>> AllCardRows;

	// ── UI 元件（動態刷新） ──────────────────────────────────────

	TSharedPtr<SScrollBox> AvailableScrollBox;
	TSharedPtr<SScrollBox> DeckScrollBox;
	TSharedPtr<STextBlock> DeckCountText;

	// ── 輔助函式 ────────────────────────────────────────────────

	// 從 DataTable 讀取所有 RowName
	void LoadAllCards();

	// 將玩家目前牌組複製到 EditingDeck
	void LoadCurrentDeck();

	// 重繪左側「所有卡牌」清單
	void RefreshAvailableCards();

	// 重繪右側「我的牌組」清單
	void RefreshDeckCards();

	// 更新牌組張數文字
	void UpdateDeckCountText();

	// 從 DataTable 取得卡牌顯示名稱 (Name + Power)
	FString GetCardDisplayName(FName RowName) const;

	// 取得 EditingDeck 中某張牌的數量
	int32 GetCardCountInEditing(FName RowName) const;

	// ── 按鈕回調 ────────────────────────────────────────────────

	FReply OnAddCard(FName RowName);
	FReply OnRemoveCard(FName RowName);
	FReply OnSave();
	FReply OnBack();

	// ── Widget 建構輔助 ──────────────────────────────────────────

	// 建立一列「可加入」的卡牌項目
	TSharedRef<SWidget> MakeAvailableCardRow(FName RowName);

	// 建立一列「已在牌組」的卡牌項目
	TSharedRef<SWidget> MakeDeckCardRow(FName RowName);

	// 建立通用按鈕
	TSharedRef<SWidget> MakeButton(const FText& Label, FOnClicked Callback,
	                               FSlateColor BgColor = FSlateColor(FLinearColor(0.2f, 0.5f, 0.9f)));
};
