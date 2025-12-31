#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DT_CardData.generated.h"

USTRUCT(BlueprintType)
struct FCardData : public FTableRowBase
{
    GENERATED_BODY()

    // 卡片名稱
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    FString Name;

    // 稀有度（例如 Common, Rare, Epic, Legendary）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    FString Rare;

    // 攻擊力或能力值
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    int32 Power;

    // 攻擊距離（若為近戰、遠程等可使用 float 或 int）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    float Range;

    // 敘述說明
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    FString Description;

    // 所屬系列（例如 "BaseSet", "Expansion1"）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    FString Series;

    // 卡牌插圖
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Visual")
    TSoftObjectPtr<UTexture2D> CardImage;

    // 卡牌背景圖
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Visual")
    TSoftObjectPtr<UTexture2D> BackgroundImage;
};
