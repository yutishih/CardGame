// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardGame/Data/DT_CardData.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "CardWidget.generated.h"

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

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

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
};
