// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CardGameHUD.h"
#include "CardGameHUDController.generated.h"

/**
 * ACardGameHUDController - 控制遊戲 HUD 的顯示
 */
UCLASS()
class CARDGAME_API ACardGameHUDController : public AHUD
{
	GENERATED_BODY()

public:
	ACardGameHUDController();

	virtual void BeginPlay() override;
	virtual void DrawHUD() override;

protected:
	// HUD Widget 類
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCardGameHUD> HUDWidgetClass;

	// HUD Widget 實例
	UPROPERTY()
	TObjectPtr<UCardGameHUD> HUDWidget;
};
