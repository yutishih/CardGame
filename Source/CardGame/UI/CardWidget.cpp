// Fill out your copyright notice in the Description page of Project Settings.

#include "CardWidget.h"
#include "Engine/Texture2D.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/PanelWidget.h"
#include "Components/SizeBox.h"
#include "Components/Button.h"
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
}

void UCardWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);

	FVector2D LocalSize = InGeometry.GetLocalSize();
	FVector2D AbsoluteSize = InGeometry.GetAbsoluteSize();
	
	UE_LOG(LogTemp, Warning, TEXT("CardWidget Hovered: Index %d. Local Size: %s, Absolute Size: %s"), 
		CardIndex, *LocalSize.ToString(), *AbsoluteSize.ToString());

	if (GEngine)
	{
		FString Msg = FString::Printf(TEXT("HOVER Index %d! Size: %s"), CardIndex, *LocalSize.ToString());
		GEngine->AddOnScreenDebugMessage(200 + CardIndex, 2.0f, FColor::Red, Msg);
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

			if (GEngine)
			{
				FString Msg = FString::Printf(TEXT("Card Index %d Size: %s (Desired: 125x175)"), CardIndex, *Brush.ImageSize.ToString());
				GEngine->AddOnScreenDebugMessage(100 + CardIndex, 1.0f, FColor::Green, Msg);
			}

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
