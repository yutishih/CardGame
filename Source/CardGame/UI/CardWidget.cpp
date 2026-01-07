// Fill out your copyright notice in the Description page of Project Settings.

#include "CardWidget.h"
#include "Engine/Texture2D.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"
#include "Components/Button.h"

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
}

void UCardWidget::SetOnClicked(FOnCardClicked InOnClicked)
{
	OnClicked = InOnClicked;
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
				
				// 設定固定大小 250x350
				const float CardWidth = 250.0f;
				const float CardHeight = 350.0f;

				// 強制設定筆刷大小，確保 UI 知道圖片尺寸
				FSlateBrush Brush = CardImage->GetBrush();
				Brush.ImageSize = FVector2D(CardWidth, CardHeight);
				CardImage->SetBrush(Brush);

				// 嘗試設定 Canvas Slot 大小 (如果是 Canvas Panel 的子物件)
				if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(CardImage->Slot))
				{
					CanvasSlot->SetSize(FVector2D(CardWidth, CardHeight));
					UE_LOG(LogTemp, Warning, TEXT("CardWidget: Set Canvas Slot Size to %fx%f"), CardWidth, CardHeight);
				}

				// 嘗試設定 SizeBox 大小 (如果是 SizeBox 的子物件，或是上層有 SizeBox)
				UPanelWidget* CurrentParent = CardImage->GetParent();
				bool bFoundSizeBox = false;
				while (CurrentParent)
				{
					UE_LOG(LogTemp, Warning, TEXT("CardWidget: Checking Parent: %s"), *CurrentParent->GetClass()->GetName());
					if (USizeBox* SizeBox = Cast<USizeBox>(CurrentParent))
					{
						SizeBox->SetWidthOverride(CardWidth);
						SizeBox->SetHeightOverride(CardHeight);
						SizeBox->SetMinDesiredWidth(CardWidth);
						SizeBox->SetMinDesiredHeight(CardHeight);
						UE_LOG(LogTemp, Warning, TEXT("CardWidget: Forced SizeBox overrides (Width/Height/Min) to %fx%f"), CardWidth, CardHeight);
						bFoundSizeBox = true;
						break;
					}
					CurrentParent = CurrentParent->GetParent();
				}

				if (!bFoundSizeBox)
				{
					UE_LOG(LogTemp, Warning, TEXT("CardWidget: No SizeBox found in parent hierarchy for CardImage."));
				}

				CardImage->SetBrushTintColor(FLinearColor::White);
				CardImage->SetColorAndOpacity(FLinearColor::White);
				CardImage->SetVisibility(ESlateVisibility::Visible);
				
				UE_LOG(LogTemp, Warning, TEXT("CardWidget: Texture loaded. Size: %dx%d. Visibility: %s"), 
					Texture->GetSizeX(), 
					Texture->GetSizeY(),
					*UEnum::GetValueAsString(CardImage->GetVisibility())
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
			// 如果沒有圖片，可以隱藏或顯示預設圖
			// CardImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CardWidget: CardImage widget NOT found! Check WBP_Card naming and IsVariable."));
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
}

void UCardWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	static float LogTimer = 0.0f;
	LogTimer += InDeltaTime;

	if (LogTimer >= 1.0f) // Log every 1 second
	{
		LogTimer = 0.0f;
		
		FVector2D WidgetSize = MyGeometry.GetLocalSize();
		UE_LOG(LogTemp, Warning, TEXT("CardWidget Debug: Entire Widget Size: %s"), *WidgetSize.ToString());

		if (CardImage)
		{
			FVector2D LocalSize = CardImage->GetCachedGeometry().GetLocalSize();
			FVector2D AbsoluteSize = CardImage->GetCachedGeometry().GetAbsoluteSize();
			
			UE_LOG(LogTemp, Warning, TEXT("CardWidget Debug: CardImage Size: %s, Absolute Size: %s, Visibility: %s, RenderOpacity: %f"), 
				*LocalSize.ToString(), 
				*AbsoluteSize.ToString(),
				*UEnum::GetValueAsString(CardImage->GetVisibility()),
				CardImage->GetRenderOpacity()
			);

			// Check if Brush size is correct
			FSlateBrush Brush = CardImage->GetBrush();
			UE_LOG(LogTemp, Warning, TEXT("CardWidget Debug: Brush ImageSize: %s, ResourceObject: %s"), 
				*Brush.ImageSize.ToString(),
				Brush.GetResourceObject() ? *Brush.GetResourceObject()->GetName() : TEXT("NULL")
			);

			// Log Parent and Slot info
			if (CardImage->GetParent())
			{
				UE_LOG(LogTemp, Warning, TEXT("CardWidget Debug: CardImage Parent: %s, Slot Type: %s"), 
					*CardImage->GetParent()->GetClass()->GetName(),
					CardImage->Slot ? *CardImage->Slot->GetClass()->GetName() : TEXT("NULL")
				);

				// Traverse up to debug hierarchy sizes
				UPanelWidget* Current = CardImage->GetParent();
				while (Current)
				{
					FVector2D CurSize = Current->GetCachedGeometry().GetLocalSize();
					UE_LOG(LogTemp, Warning, TEXT("CardWidget Debug: Hierarchy: %s, Size: %s"), *Current->GetClass()->GetName(), *CurSize.ToString());
					
					if (USizeBox* SB = Cast<USizeBox>(Current))
					{
						UE_LOG(LogTemp, Warning, TEXT("CardWidget Debug: SizeBox Settings - WidthOverride: %f, HeightOverride: %f"), SB->GetWidthOverride(), SB->GetHeightOverride());
					}

					if (Current == GetRootWidget())
					{
						UE_LOG(LogTemp, Warning, TEXT("CardWidget Debug: Reached RootWidget"));
						break;
					}
					Current = Current->GetParent();
				}
			}
		}
	}
}
