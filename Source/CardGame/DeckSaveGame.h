#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DeckSaveGame.generated.h"

/**
 * UDeckSaveGame
 * 負責將玩家自訂牌組持久化到硬碟
 * 儲存 DataTable 的 RowName 陣列
 */
UCLASS()
class CARDGAME_API UDeckSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 玩家選擇的牌組（DataTable Row Names，例如 "1", "2", ...）
	UPROPERTY(SaveGame)
	TArray<FName> PlayerDeckRowNames;

	// 存檔槽名稱
	static const FString SaveSlotName;

	// 存檔使用者索引（本地單人用 0）
	static const int32 SaveUserIndex;
};
