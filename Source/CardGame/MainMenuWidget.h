// MainMenuWidget.h
// 主選單 UI Widget

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class SMainMenuWidget : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SMainMenuWidget) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// 按鈕點擊回調
	FReply OnStartClicked();
	FReply OnDeckClicked();
	FReply OnQuitClicked();

	// 創建按鈕樣式
	TSharedRef<SWidget> CreateMenuButton(const FText& ButtonText, FOnClicked OnClicked);
};
