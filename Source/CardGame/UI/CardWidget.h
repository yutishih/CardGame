// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardGame/Data/DT_CardData.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "CardWidget.generated.h"

DECLARE_DELEGATE_OneParam(FOnCardClicked, int32);

/**
 * UCardWidget
 * 用於顯示單張卡牌資訊的 UI Widget
 */
UCLASS()
class CARDGAME_API UCardWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	// 更新卡牌顯示
	UFUNCTION(BlueprintCallable, Category = "Card")
	void UpdateCardDisplay(const FCardData& CardData);

	// 強制設置卡牌大小
	void ForceCardSize();

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

	// 設定發光效果（供 HUD 在 Hover 狀態下控制）
	UFUNCTION(BlueprintCallable, Category = "Card")
	void SetGlowEffectEnabled(bool bEnabled);

protected:
	// 綁定 UI 元件 (需要在 Widget Blueprint 中建立同名的元件)
	
	// 卡牌名稱
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* NameText;

	// 攻擊力/數值
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PowerText;

	// 卡牌敘述
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DescriptionText;

	// 卡牌圖片
	UPROPERTY(meta = (BindWidget))
	class UImage* CardImage;
    
    // 背景圖 (可選)
    UPROPERTY(meta = (BindWidgetOptional))
    class UImage* BackgroundImage;

	// 點擊按鈕 (需要覆蓋在整個卡片上)
	UPROPERTY(meta = (BindWidgetOptional))
	class UButton* ClickButton;

	// 點擊事件處理
	UFUNCTION()
	void OnCardClicked();

	// 點擊回調 (傳遞 CardWidget 本身或索引)
	FOnCardClicked OnClicked;

	// 實際套用發光外觀
	void ApplyGlowEffect();

	// 快取目前顯示資料，供拖曳視覺複製使用
	FCardData CachedCardData;
	bool bHasCachedCardData = false;

	// 是否啟用發光
	bool bGlowEffectEnabled = false;

public:
	// 設定點擊回調
	void SetOnClicked(FOnCardClicked InOnClicked);

	// 設定是否可被拖曳（通常只有玩家手牌可拖）
	void SetDraggable(bool bInDraggable);
	
	// 儲存卡牌索引，方便回傳
	int32 CardIndex = -1;

	// 是否允許拖曳
	bool bIsDraggable = false;
};
