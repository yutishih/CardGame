// Fill out your copyright notice in the Description page of Project Settings.

#include "CardWidget.h"
#include "Engine/Texture2D.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"
#include "Components/Button.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "InputCoreTypes.h"
#include "CardDragDropOperation.h"
#include "Engine/Engine.h"

void UCardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ClickButton)
	{
		UE_LOG(LogTemp, Warning, TEXT("CardWidget: ClickButton found and bound!"));
		ClickButton->OnClicked.AddDynamic(this, &UCardWidget::OnCardClicked);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CardWidget: ClickButton NOT found! Check WBP_Card naming."));
	}

	ForceCardSize();
}

FReply UCardWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDraggable && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UCardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bIsDraggable && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UCardWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!bIsDraggable)
	{
		return;
	}

	UCardDragDropOperation* DragOperation = NewObject<UCardDragDropOperation>();
	if (!DragOperation)
	{
		return;
	}

	DragOperation->CardIndex = CardIndex;
	DragOperation->Payload = this;
	DragOperation->Pivot = EDragPivot::CenterCenter;

	UCardWidget* DragVisual = nullptr;
	if (APlayerController* OwningPC = GetOwningPlayer())
	{
		DragVisual = CreateWidget<UCardWidget>(OwningPC, GetClass());
	}
	else
	{
		DragVisual = CreateWidget<UCardWidget>(GetWorld(), GetClass());
	}

	if (DragVisual)
	{
		DragVisual->CardIndex = CardIndex;
		DragVisual->SetDraggable(false);
		DragVisual->SetIsEnabled(false);
		DragVisual->SetRenderOpacity(0.9f);

		if (bHasCachedCardData)
		{
			DragVisual->UpdateCardDisplay(CachedCardData);
		}

		DragOperation->DefaultDragVisual = DragVisual;
	}
	else
	{
		DragOperation->DefaultDragVisual = nullptr;
	}

	OutOperation = DragOperation;
}

void UCardWidget::SetOnClicked(FOnCardClicked InOnClicked)
{
	OnClicked = InOnClicked;
}

void UCardWidget::SetDraggable(bool bInDraggable)
{
	bIsDraggable = bInDraggable;
}

void UCardWidget::OnCardClicked()
{
	if (OnClicked.IsBound())
	{
		OnClicked.Execute(CardIndex);
		UE_LOG(LogTemp, Warning, TEXT("CardWidget: Clicked! Index: %d"), CardIndex);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CardWidget: Clicked but no delegate bound! Index: %d"), CardIndex);
	}
}

void UCardWidget::UpdateCardDisplay(const FCardData& CardData)
{
	CachedCardData = CardData;
	bHasCachedCardData = true;

	// 更新名稱
	if (NameText)
	{
		NameText->SetText(FText::FromString(CardData.Name));
	}

	// 更新數值
	if (PowerText)
	{
		PowerText->SetText(FText::AsNumber(CardData.Power));
	}

	// 更新敘述
	if (DescriptionText)
	{
		DescriptionText->SetText(FText::FromString(CardData.Description));
	}

	// 更新卡牌圖片
	if (CardImage)
	{
		UE_LOG(LogTemp, Warning, TEXT("CardWidget: CardImage widget found."));
		if (!CardData.CardImage.IsNull())
		{
			UE_LOG(LogTemp, Warning, TEXT("CardWidget: Loading texture for card %s..."), *CardData.Name);
			UTexture2D* Texture = CardData.CardImage.LoadSynchronous();
			if (Texture)
			{
				CardImage->SetBrushFromTexture(Texture);
				
				ForceCardSize();

				CardImage->SetBrushTintColor(FLinearColor::White);
				CardImage->SetColorAndOpacity(FLinearColor::White);
				CardImage->SetVisibility(ESlateVisibility::Visible);
				
				UE_LOG(LogTemp, Warning, TEXT("CardWidget: Texture loaded. Size: %dx%d."), 
					Texture->GetSizeX(), 
					Texture->GetSizeY()
				);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("CardWidget: Failed to load texture!"));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("CardWidget: CardData.CardImage is NULL for card %s!"), *CardData.Name);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CardWidget: CardImage widget NOT found! Check WBP_Card naming."));
	}

	// 更新背景圖片
	if (BackgroundImage)
	{
		if (!CardData.BackgroundImage.IsNull())
		{
			UTexture2D* BgTexture = CardData.BackgroundImage.LoadSynchronous();
			if (BgTexture)
			{
				BackgroundImage->SetBrushFromTexture(BgTexture);
			}
		}
	}

	// 每次更新資料後，重新套用目前的發光狀態（避免被重設）
	ApplyGlowEffect();
}

void UCardWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
}

void UCardWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UCardWidget::SetGlowEffectEnabled(bool bEnabled)
{
	bGlowEffectEnabled = bEnabled;
	ApplyGlowEffect();
}

void UCardWidget::ApplyGlowEffect()
{
	if (bGlowEffectEnabled)
	{
		if (BackgroundImage)
		{
			BackgroundImage->SetColorAndOpacity(FLinearColor(1.2f, 1.2f, 0.8f, 1.0f));
		}

		if (CardImage)
		{
			CardImage->SetColorAndOpacity(FLinearColor(1.1f, 1.1f, 1.0f, 1.0f));
		}

		if (ClickButton)
		{
			ClickButton->SetBackgroundColor(FLinearColor(1.1f, 1.1f, 1.0f, 1.0f));
		}
	}
	else
	{
		if (BackgroundImage)
		{
			BackgroundImage->SetColorAndOpacity(FLinearColor::White);
		}

		if (CardImage)
		{
			CardImage->SetColorAndOpacity(FLinearColor::White);
		}

		if (ClickButton)
		{
			ClickButton->SetBackgroundColor(FLinearColor::White);
		}
	}
}

void UCardWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UCardWidget::ForceCardSize()
{
	if (!CardImage) return;

	// Note: 125x175 is 50% of 250x350
	const float CardWidth = 125.0f;
	const float CardHeight = 175.0f;
	
	// 設定圖片區域高度為使用者指定的 80 (原先 160 的 50%)
	const float ImageHeight = 80.0f; 

	// Force Brush Size
	FSlateBrush Brush = CardImage->GetBrush();
	Brush.ImageSize = FVector2D(CardWidth, ImageHeight);
	CardImage->SetBrush(Brush);

	// Try Canvas Slot
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(CardImage->Slot))
	{
		CanvasSlot->SetSize(FVector2D(CardWidth, ImageHeight));
	}

	// Try SizeBox (Parent hierarchy)
	UPanelWidget* CurrentParent = CardImage->GetParent();
	bool bIsInnerBox = true; // 標記是否為最內層的 Box (包覆圖片用的)

	while (CurrentParent)
	{
		if (USizeBox* SizeBox = Cast<USizeBox>(CurrentParent))
		{
			SizeBox->SetWidthOverride(CardWidth);
			
			if (bIsInnerBox)
			{
				// 內層 SizeBox：只設定為圖片高度，避免推擠文字
				SizeBox->SetHeightOverride(ImageHeight);
				SizeBox->SetMinDesiredHeight(ImageHeight);
				bIsInnerBox = false; // 下一個找到的 SizeBox 就是外層了
				UE_LOG(LogTemp, Warning, TEXT("CardWidget: Inner SizeBox set to %fx%f"), CardWidth, ImageHeight);
			}
			else
			{
				// 外層 SizeBox (Root)：設定為完整卡牌高度
				SizeBox->SetHeightOverride(CardHeight);
				SizeBox->SetMinDesiredHeight(CardHeight);
				UE_LOG(LogTemp, Warning, TEXT("CardWidget: Outer SizeBox set to %fx%f"), CardWidth, CardHeight);
			}

			SizeBox->SetMinDesiredWidth(CardWidth);
		}
		CurrentParent = CurrentParent->GetParent();
	}
}
