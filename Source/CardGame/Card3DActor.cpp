// Card3DActor.cpp

#include "Card3DActor.h"
#include "CardTableManager.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

ACard3DActor::ACard3DActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	// 啟用輸入事件 - 這對於滑鼠懸停和點擊是必要的
	AutoReceiveInput = EAutoReceiveInput::Player0;

	// 創建根組件
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootSceneComponent;

	// 創建卡牌正面網格（使用立方體，較大尺寸便於看到和點擊）
	CardMeshFront = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CardMeshFront"));
	CardMeshFront->SetupAttachment(RootComponent);
	CardMeshFront->SetRelativeScale3D(FVector(40.0f, 60.0f, 2.0f)); // 卡牌尺寸 (40x60x2 單位)
	CardMeshFront->SetRelativeLocation(FVector(0, 0, 1.0f));
	
	// 使用引擎內建的立方體作為卡牌基礎
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		CardMeshFront->SetStaticMesh(CubeMesh.Object);
	}

	// 創建卡牌背面網格
	CardMeshBack = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CardMeshBack"));
	CardMeshBack->SetupAttachment(RootComponent);
	CardMeshBack->SetRelativeScale3D(FVector(40.0f, 60.0f, 2.0f));
	CardMeshBack->SetRelativeLocation(FVector(0, 0, -1.0f));
	CardMeshBack->SetRelativeRotation(FRotator(0, 0, 180));
	
	if (CubeMesh.Succeeded())
	{
		CardMeshBack->SetStaticMesh(CubeMesh.Object);
	}

	// 創建數值文字
	CardValueText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("CardValueText"));
	CardValueText->SetupAttachment(CardMeshFront);
	CardValueText->SetRelativeLocation(FVector(0, 0, 3.0f));
	CardValueText->SetRelativeRotation(FRotator(90, 0, 180));
	CardValueText->SetHorizontalAlignment(EHTA_Center);
	CardValueText->SetVerticalAlignment(EVRTA_TextCenter);
	CardValueText->SetWorldSize(50.0f);
	CardValueText->SetTextRenderColor(FColor::White);
	CardValueText->SetText(FText::FromString(TEXT("?")));

	// 初始化變數
	OwnerPlayerId = 0;
	HandIndex = -1;
	CurrentSide = ECardSide::Back;
	bIsAnimating = false;
	bIsFlipping = false;
	bIsMoving = false;
	FlipProgress = 0.0f;
	FlipDuration = 0.3f;
	bFlipToFront = false;
	MoveProgress = 0.0f;
	MoveDuration = 0.5f;
	bIsHovered = false;
	OriginalLocation = FVector::ZeroVector;
}

void ACard3DActor::BeginPlay()
{
	Super::BeginPlay();
	
	CreateDefaultMaterials();
	UpdateCardDisplay();
	
	// 啟用滑鼠點擊和懸停事件 - 這些是關鍵設置
	SetActorEnableCollision(true);
	
	// 確保不阻擋輸入
	bBlockInput = false;
	
	// 啟用游標和點擊事件 - 使用正確的碰撞配置
	if (CardMeshFront)
	{
		CardMeshFront->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CardMeshFront->SetCollisionObjectType(ECC_WorldDynamic);
		CardMeshFront->SetCollisionResponseToAllChannels(ECR_Ignore);
		CardMeshFront->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		CardMeshFront->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
	if (CardMeshBack)
	{
		CardMeshBack->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CardMeshBack->SetCollisionObjectType(ECC_WorldDynamic);
		CardMeshBack->SetCollisionResponseToAllChannels(ECR_Ignore);
		CardMeshBack->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
		CardMeshBack->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	}
	
	UE_LOG(LogTemp, Warning, TEXT("Card3DActor BeginPlay - Owner: %d, HandIndex: %d, Location: %s"), 
		OwnerPlayerId, HandIndex, *GetActorLocation().ToString());
}

void ACard3DActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsFlipping)
	{
		UpdateFlipAnimation(DeltaTime);
	}

	if (bIsMoving)
	{
		UpdateMoveAnimation(DeltaTime);
	}

	bIsAnimating = bIsFlipping || bIsMoving;
}

void ACard3DActor::SetCardData(const FCard& InCard)
{
	CardData = InCard;
	UpdateCardDisplay();
}

void ACard3DActor::SetOwnerPlayer(int32 PlayerId)
{
	OwnerPlayerId = PlayerId;
	UpdateCardDisplay();
}

void ACard3DActor::FlipCard(bool bShowFront, bool bAnimate)
{
	ECardSide TargetSide = bShowFront ? ECardSide::Front : ECardSide::Back;
	
	if (CurrentSide == TargetSide)
	{
		return;
	}

	bFlipToFront = bShowFront;

	if (bAnimate)
	{
		bIsFlipping = true;
		FlipProgress = 0.0f;
	}
	else
	{
		// 立即翻轉
		CurrentSide = TargetSide;
		FRotator NewRotation = bShowFront ? FRotator(0, 0, 0) : FRotator(180, 0, 0);
		SetActorRotation(NewRotation);
		UpdateCardDisplay();
	}
}

void ACard3DActor::MoveToLocation(FVector TargetLocation, float Duration)
{
	MoveStartLocation = GetActorLocation();
	MoveTargetLocation = TargetLocation;
	MoveDuration = Duration;
	MoveProgress = 0.0f;
	bIsMoving = true;
}

void ACard3DActor::SetHighlight(bool bHighlight)
{
	if (FrontMaterial)
	{
		if (bHighlight)
		{
			FrontMaterial->SetScalarParameterValue(TEXT("Emissive"), 0.5f);
		}
		else
		{
			FrontMaterial->SetScalarParameterValue(TEXT("Emissive"), 0.0f);
		}
	}
	
	// 也可以稍微提升卡牌位置
	if (bHighlight)
	{
		AddActorLocalOffset(FVector(0, 0, 10.0f));
	}
	else
	{
		AddActorLocalOffset(FVector(0, 0, -10.0f));
	}
}

void ACard3DActor::UpdateFlipAnimation(float DeltaTime)
{
	FlipProgress += DeltaTime / FlipDuration;

	if (FlipProgress >= 1.0f)
	{
		FlipProgress = 1.0f;
		bIsFlipping = false;
		CurrentSide = bFlipToFront ? ECardSide::Front : ECardSide::Back;
	}

	// 使用正弦曲線做平滑翻轉
	float RotationAngle = bFlipToFront ? 
		(1.0f - FlipProgress) * 180.0f : 
		FlipProgress * 180.0f;

	SetActorRotation(FRotator(RotationAngle, 0, 0));

	// 在翻轉到一半時更新顯示
	if (FlipProgress >= 0.5f && CurrentSide != (bFlipToFront ? ECardSide::Front : ECardSide::Back))
	{
		CurrentSide = bFlipToFront ? ECardSide::Front : ECardSide::Back;
		UpdateCardDisplay();
	}
}

void ACard3DActor::UpdateMoveAnimation(float DeltaTime)
{
	MoveProgress += DeltaTime / MoveDuration;

	if (MoveProgress >= 1.0f)
	{
		MoveProgress = 1.0f;
		bIsMoving = false;
		SetActorLocation(MoveTargetLocation);
	}
	else
	{
		// 使用 ease-out 曲線
		float EasedProgress = 1.0f - FMath::Pow(1.0f - MoveProgress, 3.0f);
		FVector NewLocation = FMath::Lerp(MoveStartLocation, MoveTargetLocation, EasedProgress);
		SetActorLocation(NewLocation);
	}
}

void ACard3DActor::UpdateCardDisplay()
{
	if (!CardValueText)
	{
		return;
	}

	// 更新數值顯示
	if (CurrentSide == ECardSide::Front && CardData.IsValid())
	{
		CardValueText->SetText(FText::FromString(FString::Printf(TEXT("%d"), CardData.CardValue)));
		CardValueText->SetVisibility(true);
	}
	else
	{
		CardValueText->SetText(FText::FromString(TEXT("")));
		CardValueText->SetVisibility(false);
	}

	// 根據擁有者設置顏色
	if (FrontMaterial)
	{
		if (OwnerPlayerId == 0)
		{
			// 玩家卡牌 - 綠色
			FrontMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.2f, 0.6f, 0.3f, 1.0f));
		}
		else
		{
			// 對手卡牌 - 紅色
			FrontMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.6f, 0.2f, 0.2f, 1.0f));
		}
	}
}

void ACard3DActor::CreateDefaultMaterials()
{
	// 創建正面材質
	UMaterial* BaseMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	
	if (BaseMaterial)
	{
		FrontMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		if (FrontMaterial && CardMeshFront)
		{
			FrontMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.2f, 0.6f, 0.3f, 1.0f));
			CardMeshFront->SetMaterial(0, FrontMaterial);
		}
		
		BackMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		if (BackMaterial && CardMeshBack)
		{
			// 卡牌背面 - 深藍色
			BackMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.1f, 0.1f, 0.4f, 1.0f));
			CardMeshBack->SetMaterial(0, BackMaterial);
		}
	}
}

void ACard3DActor::NotifyActorBeginCursorOver()
{
	Super::NotifyActorBeginCursorOver();
	
	UE_LOG(LogTemp, Warning, TEXT("Card cursor over detected! Owner: %d, HandIndex: %d, Animating: %d"), OwnerPlayerId, HandIndex, bIsAnimating);

	// 只有玩家的手牌可以高亮（HandIndex >= 0 表示在手牌中）
	if (OwnerPlayerId == 0 && HandIndex >= 0 && !bIsAnimating)
	{
		bIsHovered = true;
		
		// 卡牌上升效果 - 使用 AddActorWorldOffset
		AddActorWorldOffset(FVector(0, 0, 30.0f));
		
		// 稍微放大卡牌
		SetActorScale3D(FVector(1.2f, 1.2f, 1.2f));
		
		// 發光效果 - 變成更亮的顏色
		if (FrontMaterial)
		{
			FrontMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.5f, 1.0f, 0.6f, 1.0f));
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Card hover begin: index %d"), HandIndex);
	}
}

void ACard3DActor::NotifyActorEndCursorOver()
{
	Super::NotifyActorEndCursorOver();

	if (bIsHovered)
	{
		bIsHovered = false;
		
		// 卡牌回到原位
		AddActorWorldOffset(FVector(0, 0, -30.0f));
		
		// 恢復原本大小
		SetActorScale3D(FVector(1.0f, 1.0f, 1.0f));
		
		// 恢復原本顏色
		if (FrontMaterial)
		{
			FrontMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.2f, 0.6f, 0.3f, 1.0f));
		}
		
		UE_LOG(LogTemp, Warning, TEXT("Card hover end: index %d"), HandIndex);
	}
}

void ACard3DActor::NotifyActorOnClicked(FKey ButtonPressed)
{
	Super::NotifyActorOnClicked(ButtonPressed);

	// 只有玩家的手牌可以點擊出牌
	if (OwnerPlayerId == 0 && HandIndex >= 0 && !bIsAnimating)
	{
		UE_LOG(LogTemp, Warning, TEXT("3D Card clicked: index %d, value %d"), HandIndex, CardData.CardValue);
		
		if (CardTableManager)
		{
			CardTableManager->OnCardClicked(this);
		}
	}
}
