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
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SizeBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/SlateTypes.h"
#include "Widgets/SWidget.h"

namespace
{
UStaticMesh* LoadFirstAvailableInventoryMesh(const TCHAR* PrimaryPath, const TCHAR* FallbackPath = nullptr)
{
	if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, PrimaryPath))
	{
		return Mesh;
	}

	return FallbackPath ? LoadObject<UStaticMesh>(nullptr, FallbackPath) : nullptr;
}

bool IsStackableInventoryItem(EGreenhouseInventoryItem Item)
{
	return Item == EGreenhouseInventoryItem::Lily
		|| Item == EGreenhouseInventoryItem::EmptyPot
		|| Item == EGreenhouseInventoryItem::SoilBag
		|| Item == EGreenhouseInventoryItem::FertilizerBag;
}

bool IsUniqueInventoryItem(EGreenhouseInventoryItem Item)
{
	return Item == EGreenhouseInventoryItem::WateringCan
		|| Item == EGreenhouseInventoryItem::Trowel
		|| Item == EGreenhouseInventoryItem::Secateur;
}

constexpr float InventoryPreviewFillWorldSize = 88.0f;
const FVector InventoryPreviewCameraLocation(-145.0f, 0.0f, 8.0f);

FRotator GetInventoryPreviewRotation(EGreenhouseInventoryItem Item)
{
	switch (Item)
	{
	case EGreenhouseInventoryItem::Lily:
		return FRotator(0.0f, 180.0f, 0.0f);
	case EGreenhouseInventoryItem::WateringCan:
		return FRotator(0.0f, 35.0f, 0.0f);
	case EGreenhouseInventoryItem::EmptyPot:
		return FRotator(0.0f, 180.0f, 0.0f);
	case EGreenhouseInventoryItem::SoilBag:
	case EGreenhouseInventoryItem::FertilizerBag:
		return FRotator(0.0f, 90.0f, 0.0f);
	case EGreenhouseInventoryItem::Trowel:
		return FRotator(0.0f, 135.0f, -18.0f);
	case EGreenhouseInventoryItem::Secateur:
		return FRotator(0.0f, 130.0f, -8.0f);
	default:
		return FRotator::ZeroRotator;
	}
}

FBox BuildRotatedMeshBox(const FBox& LocalBox, const FRotator& Rotation)
{
	FBox RotatedBox(ForceInit);
	for (int32 X = 0; X < 2; ++X)
	{
		for (int32 Y = 0; Y < 2; ++Y)
		{
			for (int32 Z = 0; Z < 2; ++Z)
			{
				const FVector Corner(
					X == 0 ? LocalBox.Min.X : LocalBox.Max.X,
					Y == 0 ? LocalBox.Min.Y : LocalBox.Max.Y,
					Z == 0 ? LocalBox.Min.Z : LocalBox.Max.Z);
				RotatedBox += Rotation.RotateVector(Corner);
			}
		}
	}

	return RotatedBox;
}
}

AGreenhouseItemPreviewActor::AGreenhouseItemPreviewActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewMesh"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCastShadow(false);

	KeyLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PreviewKeyLight"));
	KeyLightComponent->SetupAttachment(SceneRoot);
	KeyLightComponent->SetRelativeLocation(FVector(-96.0f, -44.0f, 46.0f));
	KeyLightComponent->SetIntensity(250.0f);
	KeyLightComponent->SetAttenuationRadius(380.0f);
	KeyLightComponent->SetCastShadows(false);
	KeyLightComponent->SetLightColor(FLinearColor(1.0f, 0.96f, 0.88f));

	FillLightComponent = CreateDefaultSubobject<UPointLightComponent>(TEXT("PreviewFillLight"));
	FillLightComponent->SetupAttachment(SceneRoot);
	FillLightComponent->SetRelativeLocation(FVector(-92.0f, 58.0f, 18.0f));
	FillLightComponent->SetIntensity(190.0f);
	FillLightComponent->SetAttenuationRadius(380.0f);
	FillLightComponent->SetCastShadows(false);
	FillLightComponent->SetLightColor(FLinearColor(0.72f, 0.82f, 1.0f));

	LilyMesh = LoadFirstAvailableInventoryMesh(TEXT("/Game/models/Plants/Lily/lily.lily"));
	WateringCanMesh = LoadFirstAvailableInventoryMesh(
		TEXT("/Game/models/equipment/Watering_Can/watering_can.watering_can"),
		TEXT("/Game/models/furniture/Watering_Can/watering_can.watering_can"));
	EmptyPotMesh = LoadFirstAvailableInventoryMesh(
		TEXT("/Game/models/equipment/pots/ornament_plants/empty/ornament_plants_pot_empty.ornament_plants_pot_empty"),
		TEXT("/Game/models/equipment/pots/ornament plants/empty/ornament_plants_pot_empty.ornament_plants_pot_empty"));
	SoilBagMesh = LoadFirstAvailableInventoryMesh(
		TEXT("/Game/models/equipment/soil/ornament_plants/ornament_plants_soil.ornament_plants_soil"),
		TEXT("/Game/models/equipment/soil/ornament plants/ornament_plants_soil.ornament_plants_soil"));
	FertilizerBagMesh = LoadFirstAvailableInventoryMesh(
		TEXT("/Game/models/equipment/fertilizer/ornament_plants/ornament_plants_fertilizer.ornament_plants_fertilizer"));
	TrowelMesh = LoadFirstAvailableInventoryMesh(TEXT("/Game/models/equipment/Trowel/trowel.trowel"));
	SecateurMesh = LoadFirstAvailableInventoryMesh(TEXT("/Game/models/equipment/Secateur/Secateur/secateur.secateur"));

	SetPreviewItem(EGreenhouseInventoryItem::None);
}

void AGreenhouseItemPreviewActor::SetPreviewItem(EGreenhouseInventoryItem InItem)
{
	if (!MeshComponent)
	{
		return;
	}

	UStaticMesh* Mesh = nullptr;

	if (InItem == EGreenhouseInventoryItem::Lily)
	{
		Mesh = LilyMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::WateringCan)
	{
		Mesh = WateringCanMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::EmptyPot)
	{
		Mesh = EmptyPotMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::SoilBag)
	{
		Mesh = SoilBagMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::FertilizerBag)
	{
		Mesh = FertilizerBagMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::Trowel)
	{
		Mesh = TrowelMesh;
	}
	else if (InItem == EGreenhouseInventoryItem::Secateur)
	{
		Mesh = SecateurMesh;
	}

	MeshComponent->SetStaticMesh(Mesh);
	const FRotator PreviewRotation = GetInventoryPreviewRotation(InItem);
	MeshComponent->SetRelativeRotation(PreviewRotation);
	MeshComponent->SetRelativeScale3D(FVector::OneVector);

	if (Mesh)
	{
		const FBox RotatedBox = BuildRotatedMeshBox(Mesh->GetBoundingBox(), PreviewRotation);
		const FVector RotatedCenter = RotatedBox.GetCenter();
		const FVector RotatedExtent = RotatedBox.GetExtent();
		const float VisibleWidth = static_cast<float>(RotatedExtent.Y * 2.0);
		const float VisibleHeight = static_cast<float>(RotatedExtent.Z * 2.0);
		const float VisibleSize = FMath::Max3(VisibleWidth, VisibleHeight, 1.0f);
		const float PreviewScale = InventoryPreviewFillWorldSize / VisibleSize;
		MeshComponent->SetRelativeScale3D(FVector(PreviewScale));
		MeshComponent->SetRelativeLocation(-RotatedCenter * PreviewScale);
	}
	else
	{
		MeshComponent->SetRelativeLocation(FVector::ZeroVector);
	}

	SetActorHiddenInGame(InItem == EGreenhouseInventoryItem::None || Mesh == nullptr);
}

TSharedRef<SWidget> UGreenhouseItemIconWidget::RebuildWidget()
{
	TSharedRef<SWidget> Widget = Super::RebuildWidget();
	ConfigurePreviewScene();
	return Widget;
}

void UGreenhouseItemIconWidget::SetItem(EGreenhouseInventoryItem InItem)
{
	Item = InItem;
	ConfigurePreviewScene();
}

void UGreenhouseItemIconWidget::ConfigurePreviewScene()
{
	SetVisibility(Item == EGreenhouseInventoryItem::None ? ESlateVisibility::Hidden : ESlateVisibility::HitTestInvisible);
	if (!GetViewportWorld())
	{
		return;
	}

	SetBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.0f));
	SetEnableAdvancedFeatures(false);
	SetLightIntensity(0.0f);
	SetSkyIntensity(1.25f);
	SetViewLocation(InventoryPreviewCameraLocation);
	SetViewRotation(FRotator(0.0f, 0.0f, 0.0f));
	SetShowFlag(TEXT("Grid"), false);
	SetShowFlag(TEXT("Selection"), false);
	SetShowFlag(TEXT("AntiAliasing"), true);
	SetShowFlag(TEXT("Lighting"), true);
	SetShowFlag(TEXT("DirectionalLights"), false);
	SetShowFlag(TEXT("PointLights"), true);
	SetShowFlag(TEXT("SkyLighting"), true);
	SetShowFlag(TEXT("DynamicShadows"), false);
	SetShowFlag(TEXT("AmbientOcclusion"), false);
	SetShowFlag(TEXT("PostProcessing"), false);
	SetShowFlag(TEXT("Atmosphere"), false);
	SetShowFlag(TEXT("Fog"), false);
	SetShowFlag(TEXT("MotionBlur"), false);

	if (!PreviewActor)
	{
		PreviewActor = Cast<AGreenhouseItemPreviewActor>(Spawn(AGreenhouseItemPreviewActor::StaticClass()));
	}

	if (PreviewActor)
	{
		PreviewActor->SetPreviewItem(Item);
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

	CrosshairDot = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CrosshairDot"));
	CrosshairDot->SetWidthOverride(5.0f);
	CrosshairDot->SetHeightOverride(5.0f);
	CrosshairDot->SetVisibility(ESlateVisibility::HitTestInvisible);
	UBorder* CrosshairFill = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("CrosshairFill"));
	CrosshairFill->SetBrushColor(FLinearColor(0.90f, 0.98f, 0.88f, 0.86f));
	CrosshairDot->AddChild(CrosshairFill);
	UCanvasPanelSlot* CrosshairSlot = RootCanvas->AddChildToCanvas(CrosshairDot);
	CrosshairSlot->SetAnchors(FAnchors(0.5f, 0.5f));
	CrosshairSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	CrosshairSlot->SetPosition(FVector2D(0.0f, 0.0f));
	CrosshairSlot->SetAutoSize(true);
	CrosshairSlot->SetZOrder(50);

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

	UButton* EmptyPotButton = CreateDebugButton(TEXT("GiveEmptyPotButton"), FText::FromString(TEXT("Give empty pot")), GivePanel);
	EmptyPotButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveEmptyPot);

	UButton* SoilBagButton = CreateDebugButton(TEXT("GiveSoilBagButton"), FText::FromString(TEXT("Give soil bag")), GivePanel);
	SoilBagButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveSoilBag);

	UButton* FertilizerBagButton = CreateDebugButton(TEXT("GiveFertilizerBagButton"), FText::FromString(TEXT("Give fertilizer bag")), GivePanel);
	FertilizerBagButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveFertilizerBag);

	UButton* TrowelButton = CreateDebugButton(TEXT("GiveTrowelButton"), FText::FromString(TEXT("Give trowel")), GivePanel);
	TrowelButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveTrowel);

	UButton* SecateurButton = CreateDebugButton(TEXT("GiveSecateurButton"), FText::FromString(TEXT("Give secateur")), GivePanel);
	SecateurButton->OnClicked.AddDynamic(this, &UGreenhouseInventoryWidget::GiveSecateur);

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
	Button->SetBackgroundColor(FLinearColor::Transparent);

	FButtonStyle ButtonStyle = Button->GetStyle();
	ButtonStyle.SetNormal(FSlateNoResource());
	ButtonStyle.SetHovered(FSlateNoResource());
	ButtonStyle.SetPressed(FSlateNoResource());
	ButtonStyle.SetDisabled(FSlateNoResource());
	ButtonStyle.NormalPadding = FMargin(0.0f);
	ButtonStyle.PressedPadding = FMargin(0.0f);
	Button->SetStyle(ButtonStyle);

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
	Background->SetPadding(FMargin(3.0f));
	SizeBox->AddChild(Background);

	UBorder* Inner = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
	Inner->SetBrushColor(SlotIndex == INDEX_NONE ? FLinearColor(0.07f, 0.09f, 0.07f, 0.96f) : GetSlotInnerColor(SlotIndex));
	Inner->SetPadding(FMargin(0.0f));
	Background->SetContent(Inner);

	UOverlay* Overlay = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass());
	Inner->SetContent(Overlay);

	UGreenhouseItemIconWidget* Icon = WidgetTree->ConstructWidget<UGreenhouseItemIconWidget>(UGreenhouseItemIconWidget::StaticClass());
	UOverlaySlot* IconSlot = Overlay->AddChildToOverlay(Icon);
	IconSlot->SetHorizontalAlignment(HAlign_Fill);
	IconSlot->SetVerticalAlignment(VAlign_Fill);
	IconSlot->SetPadding(FMargin(0.0f));

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
	if (IsUniqueInventoryItem(Item) && HasItem(Item))
	{
		return;
	}

	if (Item == EGreenhouseInventoryItem::None)
	{
		return;
	}

	if (IsStackableInventoryItem(Item))
	{
		for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
		{
			if (Slots[SlotIndex] == Item && SlotStacks[SlotIndex] < MaxStackCount)
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
		return FLinearColor(0.62f, 0.64f, 0.62f, 1.0f);
	}

	return FLinearColor(0.42f, 0.44f, 0.42f, 1.0f);
}

FLinearColor UGreenhouseInventoryWidget::GetSlotInnerColor(int32 SlotIndex) const
{
	const bool bHotbarSlot = SlotIndex < HotbarSlotCount;
	if (bHotbarSlot && SlotIndex == SelectedHotbarSlot)
	{
		return FLinearColor(0.005f, 0.005f, 0.005f, 1.0f);
	}

	return FLinearColor(0.005f, 0.005f, 0.005f, 1.0f);
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

void UGreenhouseInventoryWidget::GiveEmptyPot()
{
	AddItem(EGreenhouseInventoryItem::EmptyPot);
}

void UGreenhouseInventoryWidget::GiveSoilBag()
{
	AddItem(EGreenhouseInventoryItem::SoilBag);
}

void UGreenhouseInventoryWidget::GiveFertilizerBag()
{
	AddItem(EGreenhouseInventoryItem::FertilizerBag);
}

void UGreenhouseInventoryWidget::GiveTrowel()
{
	AddItem(EGreenhouseInventoryItem::Trowel);
}

void UGreenhouseInventoryWidget::GiveSecateur()
{
	AddItem(EGreenhouseInventoryItem::Secateur);
}
