#include "GreenhouseInventoryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Widgets/SWidget.h"

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
	SlotTexts.SetNum(TotalSlotCount);
	SlotBackgrounds.SetNum(TotalSlotCount);

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("InventoryRoot"));
	WidgetTree->RootWidget = Root;

	UHorizontalBox* Hotbar = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("Hotbar"));
	UCanvasPanelSlot* HotbarCanvasSlot = Root->AddChildToCanvas(Hotbar);
	HotbarCanvasSlot->SetAnchors(FAnchors(0.5f, 1.0f));
	HotbarCanvasSlot->SetAlignment(FVector2D(0.5f, 1.0f));
	HotbarCanvasSlot->SetPosition(FVector2D(0.0f, -28.0f));
	HotbarCanvasSlot->SetAutoSize(true);

	for (int32 SlotIndex = 0; SlotIndex < HotbarSlotCount; ++SlotIndex)
	{
		UGreenhouseInventorySlotButton* Button = CreateSlotButton(SlotIndex, FVector2D(82.0f, 82.0f));
		UHorizontalBoxSlot* BoxSlot = Hotbar->AddChildToHorizontalBox(Button);
		BoxSlot->SetPadding(FMargin(4.0f));
	}

	UBorder* InventoryFrame = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InventoryFrame"));
	InventoryFrame->SetBrushColor(FLinearColor(0.03f, 0.035f, 0.04f, 0.86f));
	UCanvasPanelSlot* InventoryCanvasSlot = Root->AddChildToCanvas(InventoryFrame);
	InventoryCanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	InventoryCanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	InventoryCanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
	InventoryCanvasSlot->SetAutoSize(true);

	InventoryPanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InventoryPanel"));
	InventoryFrame->SetContent(InventoryPanel);

	for (int32 Row = 0; Row < InventoryRowCount; ++Row)
	{
		UHorizontalBox* RowBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass());
		UVerticalBoxSlot* RowSlot = InventoryPanel->AddChildToVerticalBox(RowBox);
		RowSlot->SetPadding(FMargin(12.0f, Row == 0 ? 14.0f : 4.0f, 12.0f, 4.0f));

		for (int32 Column = 0; Column < InventoryColumnCount; ++Column)
		{
			const int32 SlotIndex = HotbarSlotCount + Row * InventoryColumnCount + Column;
			UGreenhouseInventorySlotButton* Button = CreateSlotButton(SlotIndex, FVector2D(70.0f, 70.0f));
			UHorizontalBoxSlot* BoxSlot = RowBox->AddChildToHorizontalBox(Button);
			BoxSlot->SetPadding(FMargin(3.0f));
		}
	}

	HeldItemText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("HeldItemText"));
	HeldItemText->SetColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.92f, 0.95f, 1.0f)));
	HeldItemText->SetJustification(ETextJustify::Center);
	UVerticalBoxSlot* HeldSlot = InventoryPanel->AddChildToVerticalBox(HeldItemText);
	HeldSlot->SetPadding(FMargin(12.0f, 8.0f, 12.0f, 12.0f));

	UVerticalBox* GivePanel = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("GivePanel"));
	UCanvasPanelSlot* GiveCanvasSlot = Root->AddChildToCanvas(GivePanel);
	GiveCanvasSlot->SetAnchors(FAnchors(1.0f, 0.0f));
	GiveCanvasSlot->SetAlignment(FVector2D(1.0f, 0.0f));
	GiveCanvasSlot->SetPosition(FVector2D(-24.0f, 24.0f));
	GiveCanvasSlot->SetAutoSize(true);

	auto CreateGiveButton = [this, GivePanel](const FName Name, const FText Label)
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
		UTextBlock* LabelText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		LabelText->SetText(Label);
		LabelText->SetColorAndOpacity(FSlateColor(FLinearColor(0.04f, 0.05f, 0.055f, 1.0f)));
		LabelText->SetJustification(ETextJustify::Center);
		Button->AddChild(LabelText);

		UVerticalBoxSlot* ButtonSlot = GivePanel->AddChildToVerticalBox(Button);
		ButtonSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 6.0f));
		return Button;
	};

	UButton* LilyButton = CreateGiveButton(TEXT("GiveLilyButton"), FText::FromString(TEXT("Give lily")));
	LilyButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveLily);

	UButton* WateringCanButton = CreateGiveButton(TEXT("GiveWateringCanButton"), FText::FromString(TEXT("Give watering can")));
	WateringCanButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveWateringCan);
}

UGreenhouseInventorySlotButton* UGreenhouseInventoryWidget::CreateSlotButton(int32 SlotIndex, const FVector2D& Size)
{
	UGreenhouseInventorySlotButton* Button = WidgetTree->ConstructWidget<UGreenhouseInventorySlotButton>(UGreenhouseInventorySlotButton::StaticClass());
	Button->Setup(this, SlotIndex);

	USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
	SizeBox->SetWidthOverride(Size.X);
	SizeBox->SetHeightOverride(Size.Y);

	UBorder* Background = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Background->SetBrushColor(GetSlotColor(SlotIndex));
	SizeBox->AddChild(Background);

	UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	Text->SetJustification(ETextJustify::Center);
	Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.93f, 0.96f, 0.98f, 1.0f)));
	Text->SetAutoWrapText(true);
	Background->SetContent(Text);

	Button->AddChild(SizeBox);
	SlotTexts[SlotIndex] = Text;
	SlotBackgrounds[SlotIndex] = Background;
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

	for (EGreenhouseInventoryItem Slot : Slots)
	{
		if (Slot == Item)
		{
			return true;
		}
	}

	return false;
}

void UGreenhouseInventoryWidget::RefreshSlots()
{
	for (int32 SlotIndex = 0; SlotIndex < SlotTexts.Num(); ++SlotIndex)
	{
		if (SlotTexts[SlotIndex])
		{
			const EGreenhouseInventoryItem Item = Slots.IsValidIndex(SlotIndex) ? Slots[SlotIndex] : EGreenhouseInventoryItem::None;
			const int32 StackCount = SlotStacks.IsValidIndex(SlotIndex) ? SlotStacks[SlotIndex] : 0;
			SlotTexts[SlotIndex]->SetText(GetItemText(Item, StackCount));
		}

		if (SlotBackgrounds[SlotIndex])
		{
			SlotBackgrounds[SlotIndex]->SetBrushColor(GetSlotColor(SlotIndex));
		}
	}

	if (HeldItemText)
	{
		const bool bHoldingItem = HeldItem != EGreenhouseInventoryItem::None;
		HeldItemText->SetText(bHoldingItem ? FText::Format(FText::FromString(TEXT("Holding: {0}")), GetItemText(HeldItem, HeldItemStack)) : FText::GetEmpty());
	}
}

FText UGreenhouseInventoryWidget::GetItemText(EGreenhouseInventoryItem Item, int32 StackCount) const
{
	switch (Item)
	{
	case EGreenhouseInventoryItem::Lily:
		if (StackCount > 1)
		{
			return FText::Format(FText::FromString(TEXT("Lily\nx{0}")), FText::AsNumber(StackCount));
		}
		return FText::FromString(TEXT("Lily"));
	case EGreenhouseInventoryItem::WateringCan:
		return FText::FromString(TEXT("Watering\ncan"));
	default:
		return FText::GetEmpty();
	}
}

FLinearColor UGreenhouseInventoryWidget::GetSlotColor(int32 SlotIndex) const
{
	const bool bHotbarSlot = SlotIndex < HotbarSlotCount;
	if (bHotbarSlot && SlotIndex == SelectedHotbarSlot)
	{
		return FLinearColor(0.28f, 0.34f, 0.40f, 0.94f);
	}

	return bHotbarSlot
		? FLinearColor(0.10f, 0.12f, 0.14f, 0.86f)
		: FLinearColor(0.075f, 0.085f, 0.095f, 0.90f);
}

void UGreenhouseInventoryWidget::GiveLily()
{
	AddItem(EGreenhouseInventoryItem::Lily);
}

void UGreenhouseInventoryWidget::GiveWateringCan()
{
	AddItem(EGreenhouseInventoryItem::WateringCan);
}
