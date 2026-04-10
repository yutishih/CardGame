// MainMenuGameMode.h
// 主選單 GameMode

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

UCLASS()
class CARDGAME_API AMainMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMainMenuGameMode();

	virtual void BeginPlay() override;

	// 開啟牌組編輯器（由主選單 Deck 按鈕呼叫）
	void ShowDeckBuilder();

	// 關閉牌組編輯器（由編輯器內的返回/存檔按鈕呼叫）
	void HideDeckBuilder();

	// 牌組編輯器使用的 DataTable（在 Blueprint/Editor 中設定）
	UPROPERTY(EditDefaultsOnly, Category = "Deck")
	TObjectPtr<class UDataTable> CardDataTable;

private:
	// 主選單 Widget
	TSharedPtr<class SMainMenuWidget> MainMenuWidget;

	// 牌組編輯器 Widget
	TSharedPtr<class SDeckBuilderWidget> DeckBuilderWidget;

	// 創建主選單 UI
	void CreateMainMenu();
};
