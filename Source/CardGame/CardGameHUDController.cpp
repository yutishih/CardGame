// Copyright Epic Games, Inc. All Rights Reserved.

#include "CardGameHUDController.h"
#include "Kismet/GameplayStatics.h"
#include "CardBattle.h"

ACardGameHUDController::ACardGameHUDController()
{
}

void ACardGameHUDController::BeginPlay()
{
	Super::BeginPlay();

	// 創建 HUD Widget
	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UCardGameHUD>(GetOwningPlayerController(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToViewport();

			// 獲取 GameMode 並初始化 HUD
			ACardBattle* BattleMode = Cast<ACardBattle>(UGameplayStatics::GetGameMode(GetWorld()));
			if (BattleMode)
			{
				HUDWidget->InitializeHUD(BattleMode);
			}
		}
	}
}

void ACardGameHUDController::DrawHUD()
{
	Super::DrawHUD();

	// 可以在這裡添加額外的 Canvas 繪製
}
