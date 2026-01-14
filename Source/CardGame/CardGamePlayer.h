// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CardBattle.h"
#include "CardGamePlayer.generated.h"

/**
 * ACardGamePlayer - 玩家控制器
 * 這是一個示例類，展示如何與卡牌對戰系統交互
 */
UCLASS()
class CARDGAME_API ACardGamePlayer : public APawn
{
	GENERATED_BODY()

public:
	ACardGamePlayer();

	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	// 設置對應的遊戲對戰模式
	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void SetBattleGameMode(ACardBattle* InBattleGameMode);

	// 獲取該玩家的 ID
	UFUNCTION(BlueprintCallable, Category = "CardGame")
	int32 GetPlayerID() const { return PlayerID; }

	// 出牌
	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void PlayCard(int32 CardIndex);

	// 隨機出牌
	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void PlayRandomCard();

protected:
	// Camera Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* CameraComponent;

	// 玩家 ID (0 或 1)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CardGame")
	int32 PlayerID;

	// 對戰遊戲模式指針
	UPROPERTY()
	ACardBattle* BattleGameMode;

	// 輸入回調
	void HandleInput_PlayCard1();
	void HandleInput_PlayCard2();
	void HandleInput_PlayCard3();
	void HandleInput_PlayCard4();
	void HandleInput_PlayCard5();

	// Input Handlers for checking movement
	void HandleMoveForward(float Value);
	void HandleMoveRight(float Value);
};
