// Copyright Epic Games, Inc. All Rights Reserved.

#include "CardGamePlayer.h"
#include "Camera/CameraComponent.h"

ACardGamePlayer::ACardGamePlayer()
	: PlayerID(0)
	, BattleGameMode(nullptr)
{
	PrimaryActorTick.bCanEverTick = true;

	// Create Root Component
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

	// Create Camera
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(RootComponent);

	// Set position to (0, 0, 5000) and look down
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 5000.0f));
	CameraComponent->SetRelativeRotation(FRotator(-90.0f, -90.0f, 0.0f)); // Pitch -90 (down), Yaw -90 (adjust as needed, usually 0 or -90 depending on world orientation)
}

void ACardGamePlayer::BeginPlay()
{
	Super::BeginPlay();

	// 自動獲取遊戲模式
	if (!BattleGameMode)
	{
		BattleGameMode = Cast<ACardBattle>(GetWorld()->GetAuthGameMode());
		if (BattleGameMode)
		{
			UE_LOG(LogTemp, Warning, TEXT("Player %d found BattleGameMode"), PlayerID);
		}
	}
}

void ACardGamePlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (!PlayerInputComponent)
	{
		return;
	}

	// 綁定按鍵輸入 (1-5 出牌)
	PlayerInputComponent->BindAction("PlayCard1", IE_Pressed, this, &ACardGamePlayer::HandleInput_PlayCard1);
	PlayerInputComponent->BindAction("PlayCard2", IE_Pressed, this, &ACardGamePlayer::HandleInput_PlayCard2);
	PlayerInputComponent->BindAction("PlayCard3", IE_Pressed, this, &ACardGamePlayer::HandleInput_PlayCard3);
	PlayerInputComponent->BindAction("PlayCard4", IE_Pressed, this, &ACardGamePlayer::HandleInput_PlayCard4);
	PlayerInputComponent->BindAction("PlayCard5", IE_Pressed, this, &ACardGamePlayer::HandleInput_PlayCard5);
}

void ACardGamePlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACardGamePlayer::SetBattleGameMode(ACardBattle* InBattleGameMode)
{
	BattleGameMode = InBattleGameMode;
}

void ACardGamePlayer::PlayCard(int32 CardIndex)
{
	if (BattleGameMode)
	{
		BattleGameMode->PlayerPlayCard(PlayerID, CardIndex);
	}
}

void ACardGamePlayer::PlayRandomCard()
{
	if (BattleGameMode)
	{
		// 獲取玩家手牌
		const TArray<FCard>& Hand = BattleGameMode->GetPlayerHand(PlayerID);
		if (Hand.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, Hand.Num() - 1);
			PlayCard(RandomIndex);
		}
	}
}

void ACardGamePlayer::HandleInput_PlayCard1()
{
	PlayCard(0);
}

void ACardGamePlayer::HandleInput_PlayCard2()
{
	PlayCard(1);
}

void ACardGamePlayer::HandleInput_PlayCard3()
{
	PlayCard(2);
}

void ACardGamePlayer::HandleInput_PlayCard4()
{
	PlayCard(3);
}

void ACardGamePlayer::HandleInput_PlayCard5()
{
	PlayCard(4);
}
