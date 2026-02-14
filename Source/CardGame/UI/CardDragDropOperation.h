// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "CardDragDropOperation.generated.h"

/**
 * UCardDragDropOperation
 * 手牌拖曳時攜帶卡牌索引
 */
UCLASS()
class CARDGAME_API UCardDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Card")
	int32 CardIndex = -1;
};
