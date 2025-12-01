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

private:
	// 主選單 Widget
	TSharedPtr<class SMainMenuWidget> MainMenuWidget;

	// 創建主選單 UI
	void CreateMainMenu();
};
