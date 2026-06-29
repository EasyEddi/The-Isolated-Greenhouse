#include "GreenhouseInventoryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SWidget.h"

class SGreenhouseItemIcon : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SGreenhouseItemIcon) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
	}

	void SetItem(EGreenhouseInventoryItem InItem)
	{
		Item = InItem;
	}

	virtual FVector2D ComputeDesiredSize(float) const override
	{
		return FVector2D(48.0f, 48.0f);
	}

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override
	{
		if (Item == EGreenhouseInventoryItem::None)
		{
			return LayerId;
		}

		const FVector2D Size = AllottedGeometry.GetLocalSize();
		const FSlateBrush* Brush = FCoreStyle::Get().GetBrush("WhiteBrush");
		const float Unit = FMath::Min(Size.X, Size.Y);
		const FVector2D Center(Size.X * 0.5f, Size.Y * 0.5f);

		auto DrawBox = [&](const FVector2D Position, const FVector2D BoxSize, const FLinearColor& Color, int32 LayerOffset = 0)
		{
			FSlateDrawElement::MakeBox(
				OutDrawElements,
				LayerId + LayerOffset,
				AllottedGeometry.ToPaintGeometry(BoxSize, FSlateLayoutTransform(Position)),
				Brush,
				ESlateDrawEffect::None,
				Color
			);
		};

		if (Item == EGreenhouseInventoryItem::Lily)
		{
			DrawBox(FVector2D(Center.X - Unit * 0.045f, Unit * 0.30f), FVector2D(Unit * 0.09f, Unit * 0.42f), FLinearColor(0.30f, 0.56f, 0.28f, 1.0f), 1);
			DrawBox(FVector2D(Center.X - Unit * 0.30f, Unit * 0.26f), FVector2D(Unit * 0.24f, Unit * 0.15f), FLinearColor(0.43f, 0.75f, 0.35f, 1.0f), 2);
			DrawBox(FVector2D(Center.X + Unit * 0.06f, Unit * 0.22f), FVector2D(Unit * 0.26f, Unit * 0.16f), FLinearColor(0.36f, 0.68f, 0.32f, 1.0f), 2);
			DrawBox(FVector2D(Center.X - Unit * 0.21f, Unit * 0.04f), FVector2D(Unit * 0.17f, Unit * 0.17f), FLinearColor(0.92f, 0.88f, 0.56f, 1.0f), 3);
			DrawBox(FVector2D(Center.X - Unit * 0.04f, Unit * 0.00f), FVector2D(Unit * 0.18f, Unit * 0.18f), FLinearColor(0.98f, 0.95f, 0.68f, 1.0f), 3);
			DrawBox(FVector2D(Center.X + Unit * 0.12f, Unit * 0.05f), FVector2D(Unit * 0.16f, Unit * 0.16f), FLinearColor(0.88f, 0.82f, 0.50f, 1.0f), 3);
		}
		else if (Item == EGreenhouseInventoryItem::WateringCan)
		{
			DrawBox(FVector2D(Unit * 0.20f, Unit * 0.38f), FVector2D(Unit * 0.46f, Unit * 0.28f), FLinearColor(0.33f, 0.57f, 0.42f, 1.0f), 1);
			DrawBox(FVector2D(Unit * 0.27f, Unit * 0.29f), FVector2D(Unit * 0.28f, Unit * 0.12f), FLinearColor(0.44f, 0.70f, 0.53f, 1.0f), 2);
			DrawBox(FVector2D(Unit * 0.59f, Unit * 0.35f), FVector2D(Unit * 0.28f, Unit * 0.08f), FLinearColor(0.40f, 0.66f, 0.48f, 1.0f), 2);
			DrawBox(FVector2D(Unit * 0.78f, Unit * 0.30f), FVector2D(Unit * 0.09f, Unit * 0.12f), FLinearColor(0.24f, 0.33f, 0.29f, 1.0f), 3);
			DrawBox(FVector2D(Unit * 0.13f, Unit * 0.34f), FVector2D(Unit * 0.10f, Unit * 0.24f), FLinearColor(0.24f, 0.37f, 0.31f, 1.0f), 2);
			DrawBox(FVector2D(Unit * 0.26f, Unit * 0.44f), FVector2D(Unit * 0.33f, Unit * 0.08f), FLinearColor(0.61f, 0.82f, 0.66f, 1.0f), 3);
		}

		return LayerId + 4;
	}

private:
	EGreenhouseInventoryItem Item = EGreenhouseInventoryItem::None;
};

TSharedRef<SWidget> UGreenhouseItemIconWidget::RebuildWidget()
{
	SAssignNew(SlateIcon, SGreenhouseItemIcon);
	SlateIcon->SetItem(Item);
	return SlateIcon.ToSharedRef();
}

void UGreenhouseItemIconWidget::SetItem(EGreenhouseInventoryItem InItem)
{
	Item = InItem;
	if (SlateIcon.IsValid())
	{
		SlateIcon->SetItem(Item);
		SlateIcon->Invalidate(EInvalidateWidgetReason::Paint);
	}
}

void UGreenhouseInventorySlotButton::Setup(UGreenhouseInventoryWidget* InInventoryWidget, int32 InSlotIndex)
{
	InventoryWidget = InInventoryWidget;
	SlotIndex = InSlotIndex;
	OnClicked.AddUniqueDynamic(this, &UGreenhouseInventorySlotButton::HandleClicked);
}

void UGreenhouseInventorySlotButton::HandleClicked()
{
	if (InventoryWidget)
	{
		InventoryWidget->HandleSlotClicked(SlotIndex);
	}
}

TSharedRef<SWidget> UGreenhouseInventoryWidget::RebuildWidget()
{
	if (!WidgetTree)
	{
		WidgetTree = NewObject<UWidgetTree>(this, TEXT("WidgetTree"), RF_Transient);
	}

	if (!bBuilt)
	{
		BuildInterface();
		bBuilt = true;
	}

	return Super::RebuildWidget();
}

void UGreenhouseInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Slots.Num() != TotalSlotCount)
	{
		Slots.Init(EGreenhouseInventoryItem::None, TotalSlotCount);
	}
	if (SlotStacks.Num() != TotalSlotCount)
	{
		SlotStacks.Init(0, TotalSlotCount);
	}

	SetInventoryOpen(false);
	RefreshSlots();
}

void UGreenhouseInventoryWidget::BuildInterface()
{
	Slots.Init(EGreenhouseInventoryItem::None, TotalSlotCount);
	SlotStacks.Init(0, TotalSlotCount);
	SlotIcons.SetNum(TotalSlotCount);
	SlotStackTexts.SetNum(TotalSlotCount);
	SlotBackgrounds.SetNum(TotalSlotCount);

	RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("InventoryRoot"));
	WidgetTree->RootWidget = RootCanvas;

	UHorizontalBox* Hotbar = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Hotbar"));
	UCanvasPanelSlot* HotbarCanvasSlot = RootCanvas->AddChildToCanvas(Hotbar);
	HotbarCanvasSlot->SetAnchors(FAnchors(0.5f, 1.0f));
	HotbarCanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	HotbarCanvasSlot->SetPosition(FVector2D(0.0f, -30.0f));
	HotbarCanvasSlot->SetAutoSize(true);

	for (int32 SlotIndex = 0; SlotIndex < HotbarSlotCount; ++SlotIndex)
	{
		UGreenhouseInventorySlotButton* Button = CreateSlotButton(SlotIndex, FVector2D(78.0f, 78.0f));
		UHorizontalBoxSlot* BoxSlot = Hotbar->AddChildToHorizontalBox(Button);
		BoxSlot->SetPadding(FMargin(5.0f));
	}

	InventoryFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InventoryFrame"));
	InventoryFrame->SetBrushColor(FLinearColor(0.16f, 0.19f, 0.15f, 0.96f));
	InventoryFrame->SetPadding(FMargin(26.0f, 22.0f, 26.0f, 24.0f));
	UCanvasPanelSlot* InventoryCanvasSlot = RootCanvas->AddChildToCanvas(InventoryFrame);
	InventoryCanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	InventoryCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	InventoryCanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
	InventoryCanvasSlot->SetAutoSize(true);

	InventoryPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InventoryPanel"));
	InventoryFrame->SetContent(InventoryPanel);

	UTextBlock* InventoryTitle = CreateLabel(FText::FromString(TEXT("INVENTORY")), FLinearColor(0.90f, 0.86f, 0.70f, 1.0f), 20);
	UVerticalBoxSlot* TitleSlot = InventoryPanel->AddChildToVerticalBox(InventoryTitle);
	TitleSlot->SetPadding(FMargin(6.0f, 0.0f, 0.0f, 18.0f));

	for (int32 Row = 0; Row < InventoryRowCount; ++Row)
	{
		UHorizontalBox* RowBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UVerticalBoxSlot* RowSlot = InventoryPanel->AddChildToVerticalBox(RowBox);
		RowSlot->SetPadding(FMargin(0.0f, Row == 0 ? 0.0f : 8.0f, 0.0f, 0.0f));

		for (int32 Column = 0; Column < InventoryColumnCount; ++Column)
		{
			const int32 SlotIndex = HotbarSlotCount + Row * InventoryColumnCount + Column;
			UGreenhouseInventorySlotButton* Button = CreateSlotButton(SlotIndex, FVector2D(72.0f, 72.0f));
			UHorizontalBoxSlot* BoxSlot = RowBox->AddChildToHorizontalBox(Button);
			BoxSlot->SetPadding(FMargin(5.0f));
		}
	}

	DebugFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("DebugFrame"));
	DebugFrame->SetBrushColor(FLinearColor(0.025f, 0.030f, 0.032f, 0.82f));
	DebugFrame->SetPadding(FMargin(12.0f, 10.0f, 12.0f, 10.0f));
	UCanvasPanelSlot* GiveCanvasSlot = RootCanvas->AddChildToCanvas(DebugFrame);
	GiveCanvasSlot->SetAnchors(FAnchors(1.0f, 0.0f));
	GiveCanvasSlot->SetAlignment(FVector2D(1.0f, 0.0f));
	GiveCanvasSlot->SetPosition(FVector2D(-24.0f, 24.0f));
	GiveCanvasSlot->SetAutoSize(true);

	UVerticalBox* GivePanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("GivePanel"));
	DebugFrame->SetContent(GivePanel);

	UTextBlock* DebugTitle = CreateLabel(FText::FromString(TEXT("Debug Tools")), FLinearColor(0.82f, 0.87f, 0.79f, 1.0f), 13);
	UVerticalBoxSlot* DebugTitleSlot = GivePanel->AddChildToVerticalBox(DebugTitle);
	DebugTitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));

	UButton* LilyButton = CreateDebugButton(TEXT("GiveLilyButton"), FText::FromString(TEXT("Give lily")), GivePanel);
	LilyButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveLily);

	UButton* WateringCanButton = CreateDebugButton(TEXT("GiveWateringCanButton"), FText::FromString(TEXT("Give watering can")), GivePanel);
	WateringCanButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveWateringCan);

	CursorPreview = CreateSlotFrame(INDEX_NONE, FVector2D(70.0f, 70.0f));
	CursorPreview->SetVisibility(ESlateVisibility::HitTestInvisible);
	CursorPreview->SetRenderOpacity(0.92f);
	UCanvasPanelSlot* CursorSlot = RootCanvas->AddChildToCanvas(CursorPreview);
	CursorSlot->SetAutoSize(true);
	CursorSlot->SetZOrder(100);
	CursorSlot->SetPosition(FVector2D(-1000.0f, -1000.0f));
}

UGreenhouseInventorySlotButton* UGreenhouseInventoryWidget::CreateSlotButton(int32 SlotIndex, const FVector2D& Size)
{
	UGreenhouseInventorySlotButton* Button = WidgetTree->ConstructWidget<UGreenhouseInventorySlotButton>(UGreenhouseInventorySlotButton::StaticClass());
	Button->Setup(this, SlotIndex);

	Button->AddChild(CreateSlotFrame(SlotIndex, Size));
	return Button;
}

USizeBox* UGreenhouseInventoryWidget::CreateSlotFrame(int32 SlotIndex, const FVector2D& Size)
{
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SizeBox->SetWidthOverride(Size.X);
	SizeBox->SetHeightOverride(Size.Y);

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Background->SetBrushColor(SlotIndex == INDEX_NONE ? FLinearColor(0.10f, 0.13f, 0.10f, 0.88f) : GetSlotColor(SlotIndex));
	Background->SetPadding(FMargin(5.0f));
	SizeBox->AddChild(Background);

	UBorder* Inner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Inner->SetBrushColor(SlotIndex == INDEX_NONE ? FLinearColor(0.07f, 0.09f, 0.07f, 0.96f) : GetSlotInnerColor(SlotIndex));
	Inner->SetPadding(FMargin(3.0f));
	Background->SetContent(Inner);

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	Inner->SetContent(Overlay);

	UGreenhouseItemIconWidget* Icon = WidgetTree->ConstructWidget<UGreenhouseItemIconWidget>(UGreenhouseItemIconWidget::StaticClass());
	UOverlaySlot* IconSlot = Overlay->AddChildToOverlay(Icon);
	IconSlot->SetHorizontalAlignment(HAlign_Fill);
	IconSlot->SetVerticalAlignment(VAlign_Fill);
	IconSlot->SetPadding(FMargin(7.0f));

	UTextBlock* StackText = CreateLabel(FText::GetEmpty(), FLinearColor(0.94f, 0.93f, 0.82f, 1.0f), 11);
	UOverlaySlot* StackSlot = Overlay->AddChildToOverlay(StackText);
	StackSlot->SetHorizontalAlignment(HAlign_Right);
	StackSlot->SetVerticalAlignment(VAlign_Bottom);
	StackSlot->SetPadding(FMargin(0.0f, 0.0f, 3.0f, 1.0f));

	if (SlotIndex == INDEX_NONE)
	{
		CursorIcon = Icon;
		CursorStackText = StackText;
	}
	else
	{
		SlotIcons[SlotIndex] = Icon;
		SlotStackTexts[SlotIndex] = StackText;
		SlotBackgrounds[SlotIndex] = Background;
	}

	return SizeBox;
}

UTextBlock* UGreenhouseInventoryWidget::CreateLabel(const FText& Text, const FLinearColor& Color, int32 FontSize, ETextJustify::Type Justification)
{
	UTextBlock* Label = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Label->SetText(Text);
	FSlateFontInfo Font = Label->GetFont();
	Font.Size = FontSize;
	Label->SetFont(Font);
	Label->SetColorAndOpacity(FSlateColor(Color));
	Label->SetJustification(Justification);
	return Label;
}

UButton* UGreenhouseInventoryWidget::CreateDebugButton(const FName Name, const FText& Label, UVerticalBox* ParentPanel)
{
	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SizeBox->SetWidthOverride(172.0f);
	SizeBox->SetHeightOverride(34.0f);

	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);

	UBorder* ButtonBackground = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	ButtonBackground->SetBrushColor(FLinearColor(0.58f, 0.66f, 0.51f, 0.92f));
	ButtonBackground->SetPadding(FMargin(8.0f, 6.0f, 8.0f, 6.0f));

	UTextBlock* LabelText = CreateLabel(Label, FLinearColor(0.03f, 0.04f, 0.035f, 1.0f), 12);
	ButtonBackground->SetContent(LabelText);
	Button->AddChild(ButtonBackground);
	SizeBox->AddChild(Button);

	UVerticalBoxSlot* ButtonSlot = ParentPanel->AddChildToVerticalBox(SizeBox);
	ButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
	return Button;
}

void UGreenhouseInventoryWidget::ToggleInventory()
{
	SetInventoryOpen(!bInventoryOpen);
}

void UGreenhouseInventoryWidget::SetInventoryOpen(bool bOpen)
{
	bInventoryOpen = bOpen;
	if (InventoryPanel)
	{
		InventoryPanel->SetVisibility(bInventoryOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (InventoryFrame)
	{
		InventoryFrame->SetVisibility(bInventoryOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	RefreshCursorPreview();
}

void UGreenhouseInventoryWidget::HandleSlotClicked(int32 SlotIndex)
{
	if (!Slots.IsValidIndex(SlotIndex))
	{
		return;
	}

	if (HeldItem == EGreenhouseInventoryItem::None)
	{
		HeldItem = Slots[SlotIndex];
		HeldItemStack = SlotStacks.IsValidIndex(SlotIndex) ? SlotStacks[SlotIndex] : 0;
		Slots[SlotIndex] = EGreenhouseInventoryItem::None;
		SlotStacks[SlotIndex] = 0;
	}
	else
	{
		Swap(HeldItem, Slots[SlotIndex]);
		Swap(HeldItemStack, SlotStacks[SlotIndex]);
	}

	RefreshSlots();
}

void UGreenhouseInventoryWidget::SelectHotbarSlot(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= HotbarSlotCount)
	{
		return;
	}

	SelectedHotbarSlot = SlotIndex;
	RefreshSlots();
}

EGreenhouseInventoryItem UGreenhouseInventoryWidget::GetSelectedHotbarItem() const
{
	return Slots.IsValidIndex(SelectedHotbarSlot) ? Slots[SelectedHotbarSlot] : EGreenhouseInventoryItem::None;
}

bool UGreenhouseInventoryWidget::ConsumeSelectedHotbarItem(EGreenhouseInventoryItem Item)
{
	if (!Slots.IsValidIndex(SelectedHotbarSlot) || Slots[SelectedHotbarSlot] != Item)
	{
		return false;
	}

	if (SlotStacks[SelectedHotbarSlot] > 1)
	{
		--SlotStacks[SelectedHotbarSlot];
	}
	else
	{
		Slots[SelectedHotbarSlot] = EGreenhouseInventoryItem::None;
		SlotStacks[SelectedHotbarSlot] = 0;
	}

	RefreshSlots();
	return true;
}

void UGreenhouseInventoryWidget::AddItem(EGreenhouseInventoryItem Item)
{
	if (Item == EGreenhouseInventoryItem::WateringCan && HasItem(Item))
	{
		return;
	}

	if (Item == EGreenhouseInventoryItem::Lily)
	{
		for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
		{
			if (Slots[SlotIndex] == EGreenhouseInventoryItem::Lily && SlotStacks[SlotIndex] < MaxStackCount)
			{
				++SlotStacks[SlotIndex];
				RefreshSlots();
				return;
			}
		}
	}

	for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
	{
		if (Slots[SlotIndex] == EGreenhouseInventoryItem::None)
		{
			Slots[SlotIndex] = Item;
			SlotStacks[SlotIndex] = 1;
			RefreshSlots();
			return;
		}
	}
}

bool UGreenhouseInventoryWidget::HasItem(EGreenhouseInventoryItem Item) const
{
	if (HeldItem == Item)
	{
		return true;
	}

	for (EGreenhouseInventoryItem SlotItem : Slots)
	{
		if (SlotItem == Item)
		{
			return true;
		}
	}

	return false;
}

void UGreenhouseInventoryWidget::RefreshSlots()
{
	for (int32 SlotIndex = 0; SlotIndex < SlotIcons.Num(); ++SlotIndex)
	{
		const EGreenhouseInventoryItem Item = Slots.IsValidIndex(SlotIndex) ? Slots[SlotIndex] : EGreenhouseInventoryItem::None;
		const int32 StackCount = SlotStacks.IsValidIndex(SlotIndex) ? SlotStacks[SlotIndex] : 0;

		if (SlotIcons[SlotIndex])
		{
			SlotIcons[SlotIndex]->SetItem(Item);
		}
		if (SlotStackTexts[SlotIndex])
		{
			SlotStackTexts[SlotIndex]->SetText(GetStackText(Item, StackCount));
		}
		if (SlotBackgrounds[SlotIndex])
		{
			SlotBackgrounds[SlotIndex]->SetBrushColor(GetSlotColor(SlotIndex));
		}
	}

	RefreshCursorPreview();
}

void UGreenhouseInventoryWidget::RefreshCursorPreview()
{
	const bool bShowCursorItem = bInventoryOpen && HeldItem != EGreenhouseInventoryItem::None;
	if (CursorPreview)
	{
		CursorPreview->SetVisibility(bShowCursorItem ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
	if (CursorIcon)
	{
		CursorIcon->SetItem(bShowCursorItem ? HeldItem : EGreenhouseInventoryItem::None);
	}
	if (CursorStackText)
	{
		CursorStackText->SetText(bShowCursorItem ? GetStackText(HeldItem, HeldItemStack) : FText::GetEmpty());
	}
}

FText UGreenhouseInventoryWidget::GetStackText(EGreenhouseInventoryItem Item, int32 StackCount) const
{
	if (Item != EGreenhouseInventoryItem::None && StackCount > 1)
	{
		return FText::AsNumber(StackCount);
	}

	return FText::GetEmpty();
}

FLinearColor UGreenhouseInventoryWidget::GetSlotColor(int32 SlotIndex) const
{
	const bool bHotbarSlot = SlotIndex < HotbarSlotCount;
	if (bHotbarSlot && SlotIndex == SelectedHotbarSlot)
	{
		return FLinearColor(0.55f, 0.59f, 0.45f, 0.98f);
	}

	return bHotbarSlot
		? FLinearColor(0.11f, 0.13f, 0.10f, 0.94f)
		: FLinearColor(0.09f, 0.11f, 0.09f, 0.96f);
}

FLinearColor UGreenhouseInventoryWidget::GetSlotInnerColor(int32 SlotIndex) const
{
	const bool bHotbarSlot = SlotIndex < HotbarSlotCount;
	if (bHotbarSlot && SlotIndex == SelectedHotbarSlot)
	{
		return FLinearColor(0.17f, 0.21f, 0.15f, 1.0f);
	}

	return FLinearColor(0.12f, 0.15f, 0.12f, bHotbarSlot ? 0.98f : 1.0f);
}

void UGreenhouseInventoryWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!CursorPreview || CursorPreview->GetVisibility() == ESlateVisibility::Collapsed)
	{
		return;
	}

	FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(this);
	if (UCanvasPanelSlot* CursorSlot = Cast<UCanvasPanelSlot>(CursorPreview->Slot))
	{
		CursorSlot->SetPosition(MousePosition + FVector2D(14.0f, 14.0f));
	}
}

void UGreenhouseInventoryWidget::GiveLily()
{
	AddItem(EGreenhouseInventoryItem::Lily);
}

void UGreenhouseInventoryWidget::GiveWateringCan()
{
	AddItem(EGreenhouseInventoryItem::WateringCan);
}
