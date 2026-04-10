#include "DeckBuilderWidget.h"
#include "CardGame/DeckManagerSubsystem.h"
#include "CardGame/Data/DT_CardData.h"

#include "Engine/DataTable.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SOverlay.h"

// ─────────────────────────────────────────────────────────────────────────────
//  Construct
// ─────────────────────────────────────────────────────────────────────────────

void SDeckBuilderWidget::Construct(const FArguments& InArgs)
{
	// 先建空 ScrollBox，Initialize() 呼叫後再填充
	SAssignNew(AvailableScrollBox, SScrollBox);
	SAssignNew(DeckScrollBox, SScrollBox);
	SAssignNew(DeckCountText, STextBlock)
		.Text(FText::FromString(TEXT("0 / 30")))
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
		.ColorAndOpacity(FSlateColor(FLinearColor::White));

	ChildSlot
	[
		SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.05f, 0.05f, 0.1f, 0.97f))
		.Padding(0)
		[
			SNew(SVerticalBox)

			// ── 頂部工具列 ────────────────────────────────────────
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(16, 12)
			[
				SNew(SHorizontalBox)

				// 標題
				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("DECK BUILDER")))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
					.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.3f)))
				]

				// 牌組張數
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0, 0, 24, 0)
				[
					DeckCountText.ToSharedRef()
				]

				// 存檔按鈕
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4, 0)
				[
					MakeButton(
						FText::FromString(TEXT("儲存")),
						FOnClicked::CreateSP(this, &SDeckBuilderWidget::OnSave),
						FSlateColor(FLinearColor(0.1f, 0.7f, 0.3f))
					)
				]

				// 返回按鈕
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(4, 0)
				[
					MakeButton(
						FText::FromString(TEXT("返回")),
						FOnClicked::CreateSP(this, &SDeckBuilderWidget::OnBack),
						FSlateColor(FLinearColor(0.6f, 0.1f, 0.1f))
					)
				]
			]

			// ── 分隔線 ────────────────────────────────────────────
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBorder)
				.BorderBackgroundColor(FLinearColor(0.3f, 0.3f, 0.3f))
				.Padding(FMargin(0, 1))
			]

			// ── 主要內容（左右分割） ─────────────────────────────
			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SSplitter)
				.Orientation(EOrientation::Orient_Horizontal)

				// 左側：所有可用卡牌
				+ SSplitter::Slot()
				.Value(0.5f)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(12, 8, 12, 4)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("所有卡牌")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 0.8f, 1.0f)))
					]

					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(8, 0, 4, 8)
					[
						AvailableScrollBox.ToSharedRef()
					]
				]

				// 右側：我的牌組
				+ SSplitter::Slot()
				.Value(0.5f)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(12, 8, 12, 4)
					[
						SNew(STextBlock)
						.Text(FText::FromString(TEXT("我的牌組")))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.8f, 1.0f, 0.8f)))
					]

					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(4, 0, 8, 8)
					[
						DeckScrollBox.ToSharedRef()
					]
				]
			]
		]
	];
}

// ─────────────────────────────────────────────────────────────────────────────
//  Initialize
// ─────────────────────────────────────────────────────────────────────────────

void SDeckBuilderWidget::Initialize(UDataTable* InDataTable, UDeckManagerSubsystem* InDeckManager)
{
	CardDataTable = InDataTable;
	DeckManager   = InDeckManager;

	LoadAllCards();
	LoadCurrentDeck();
	RefreshAvailableCards();
	RefreshDeckCards();
	UpdateDeckCountText();
}

// ─────────────────────────────────────────────────────────────────────────────
//  資料載入
// ─────────────────────────────────────────────────────────────────────────────

void SDeckBuilderWidget::LoadAllCards()
{
	AllCardRows.Empty();

	if (!CardDataTable.IsValid()) return;

	TArray<FName> RowNames = CardDataTable->GetRowNames();
	// 依 RowName 數字排序
	RowNames.Sort([](const FName& A, const FName& B)
	{
		return FCString::Atoi(*A.ToString()) < FCString::Atoi(*B.ToString());
	});

	for (const FName& RowName : RowNames)
	{
		AllCardRows.Add(MakeShared<FName>(RowName));
	}
}

void SDeckBuilderWidget::LoadCurrentDeck()
{
	EditingDeck.Empty();
	if (DeckManager.IsValid())
	{
		EditingDeck = DeckManager->GetPlayerDeck();
	}
}

// ─────────────────────────────────────────────────────────────────────────────
//  UI 刷新
// ─────────────────────────────────────────────────────────────────────────────

void SDeckBuilderWidget::RefreshAvailableCards()
{
	if (!AvailableScrollBox.IsValid()) return;

	AvailableScrollBox->ClearChildren();

	for (const TSharedPtr<FName>& RowPtr : AllCardRows)
	{
		if (!RowPtr.IsValid()) continue;
		AvailableScrollBox->AddSlot()
		.Padding(2, 2)
		[
			MakeAvailableCardRow(*RowPtr)
		];
	}
}

void SDeckBuilderWidget::RefreshDeckCards()
{
	if (!DeckScrollBox.IsValid()) return;

	DeckScrollBox->ClearChildren();

	// 將牌組去重後顯示（每張牌只顯示一列，並標示數量）
	TArray<FName> UniqueCards;
	for (const FName& RowName : EditingDeck)
	{
		UniqueCards.AddUnique(RowName);
	}

	for (const FName& RowName : UniqueCards)
	{
		DeckScrollBox->AddSlot()
		.Padding(2, 2)
		[
			MakeDeckCardRow(RowName)
		];
	}
}

void SDeckBuilderWidget::UpdateDeckCountText()
{
	if (!DeckCountText.IsValid()) return;

	FString CountStr = FString::Printf(TEXT("%d / %d"), EditingDeck.Num(), 30);
	DeckCountText->SetText(FText::FromString(CountStr));

	// 接近上限時變色警示
	FLinearColor TextColor = EditingDeck.Num() >= 30
		? FLinearColor(1.0f, 0.4f, 0.1f)
		: FLinearColor::White;
	DeckCountText->SetColorAndOpacity(FSlateColor(TextColor));
}

// ─────────────────────────────────────────────────────────────────────────────
//  輔助查詢
// ─────────────────────────────────────────────────────────────────────────────

FString SDeckBuilderWidget::GetCardDisplayName(FName RowName) const
{
	if (CardDataTable.IsValid())
	{
		static const FString ContextString(TEXT("DeckBuilder"));
		FCardData* Data = CardDataTable->FindRow<FCardData>(RowName, ContextString, false);
		if (Data)
		{
			return FString::Printf(TEXT("%s  [%d]"), *Data->Name, Data->Power);
		}
	}
	return RowName.ToString();
}

int32 SDeckBuilderWidget::GetCardCountInEditing(FName RowName) const
{
	int32 Count = 0;
	for (const FName& N : EditingDeck)
	{
		if (N == RowName) ++Count;
	}
	return Count;
}

// ─────────────────────────────────────────────────────────────────────────────
//  按鈕回調
// ─────────────────────────────────────────────────────────────────────────────

FReply SDeckBuilderWidget::OnAddCard(FName RowName)
{
	// 超過上限
	if (EditingDeck.Num() >= 30)
	{
		UE_LOG(LogTemp, Warning, TEXT("DeckBuilder: 牌組已達 30 張上限"));
		return FReply::Handled();
	}
	// 超過同張上限（2 張）
	if (GetCardCountInEditing(RowName) >= 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("DeckBuilder: %s 已達 2 張上限"), *RowName.ToString());
		return FReply::Handled();
	}

	EditingDeck.Add(RowName);
	RefreshAvailableCards();
	RefreshDeckCards();
	UpdateDeckCountText();
	return FReply::Handled();
}

FReply SDeckBuilderWidget::OnRemoveCard(FName RowName)
{
	for (int32 i = EditingDeck.Num() - 1; i >= 0; --i)
	{
		if (EditingDeck[i] == RowName)
		{
			EditingDeck.RemoveAt(i);
			break;
		}
	}
	RefreshAvailableCards();
	RefreshDeckCards();
	UpdateDeckCountText();
	return FReply::Handled();
}

FReply SDeckBuilderWidget::OnSave()
{
	if (DeckManager.IsValid())
	{
		DeckManager->SetPlayerDeck(EditingDeck);
		DeckManager->SaveDeck();
		UE_LOG(LogTemp, Log, TEXT("DeckBuilder: 牌組已儲存，共 %d 張"), EditingDeck.Num());
	}
	OnCloseRequested.ExecuteIfBound();
	return FReply::Handled();
}

FReply SDeckBuilderWidget::OnBack()
{
	// 不儲存，直接關閉
	OnCloseRequested.ExecuteIfBound();
	return FReply::Handled();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Widget 建構輔助
// ─────────────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> SDeckBuilderWidget::MakeAvailableCardRow(FName RowName)
{
	int32 CountInDeck = GetCardCountInEditing(RowName);
	bool bCanAdd      = (EditingDeck.Num() < 30) && (CountInDeck < 2);

	FString DisplayName = GetCardDisplayName(RowName);
	FString CountLabel  = CountInDeck > 0
		? FString::Printf(TEXT("  (%d/2)"), CountInDeck)
		: TEXT("");

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.1f, 0.1f, 0.15f, 0.8f))
		.Padding(FMargin(4, 2))
		[
			SNew(SHorizontalBox)

			// [+] 按鈕
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.IsEnabled(bCanAdd)
				.Text(FText::FromString(TEXT("+")))
				.OnClicked(FOnClicked::CreateSP(this, &SDeckBuilderWidget::OnAddCard, RowName))
				.ButtonColorAndOpacity(bCanAdd
					? FLinearColor(0.15f, 0.55f, 0.2f)
					: FLinearColor(0.25f, 0.25f, 0.25f))
			]

			// 卡牌名稱 + 數量
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.Padding(8, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(DisplayName + CountLabel))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
				.ColorAndOpacity(FSlateColor(
					CountInDeck >= 2
						? FLinearColor(0.5f, 0.5f, 0.5f)
						: FLinearColor::White
				))
			]
		];
}

TSharedRef<SWidget> SDeckBuilderWidget::MakeDeckCardRow(FName RowName)
{
	int32 Count = GetCardCountInEditing(RowName);
	FString DisplayName = GetCardDisplayName(RowName);

	return SNew(SBorder)
		.BorderBackgroundColor(FLinearColor(0.05f, 0.15f, 0.05f, 0.8f))
		.Padding(FMargin(4, 2))
		[
			SNew(SHorizontalBox)

			// [-] 按鈕
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("-")))
				.OnClicked(FOnClicked::CreateSP(this, &SDeckBuilderWidget::OnRemoveCard, RowName))
				.ButtonColorAndOpacity(FLinearColor(0.6f, 0.15f, 0.1f))
			]

			// 卡牌名稱
			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			.Padding(8, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(DisplayName))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]

			// 數量標籤
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0, 0, 8, 0)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(TEXT("x%d"), Count)))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
				.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.85f, 0.3f)))
			]
		];
}

TSharedRef<SWidget> SDeckBuilderWidget::MakeButton(const FText& Label, FOnClicked Callback, FSlateColor BgColor)
{
	return SNew(SButton)
		.OnClicked(Callback)
		.ButtonColorAndOpacity(BgColor.GetSpecifiedColor())
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(80)
			.HeightOverride(40)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
				.ColorAndOpacity(FSlateColor(FLinearColor::White))
			]
		];
}
