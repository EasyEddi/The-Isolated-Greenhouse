#include "GreenhousePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "GreenhouseInventoryWidget.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"

AGreenhousePlayerController::AGreenhousePlayerController()
{
	static ConstructorHelpers::FObjectFinder<UInputMappingContext> DefaultMappingContextFinder(TEXT("/Game/Input/IMC_Default"));
	if (DefaultMappingContextFinder.Succeeded())
	{
		DefaultMappingContext = DefaultMappingContextFinder.Object;
	}

	static ConstructorHelpers::FObjectFinder<UInputMappingContext> MouseLookMappingContextFinder(TEXT("/Game/Input/IMC_MouseLook"));
	if (MouseLookMappingContextFinder.Succeeded())
	{
		MouseLookMappingContext = MouseLookMappingContextFinder.Object;
	}
}

void AGreenhousePlayerController::BeginPlay()
{
	Super::BeginPlay();

	RegisterInputMappingContexts();
	ApplyInitialSpawnView();

	InventoryWidget = CreateWidget<UGreenhouseInventoryWidget>(this, UGreenhouseInventoryWidget::StaticClass());
	if (InventoryWidget)
	{
		InventoryWidget->AddToViewport();
		ApplyInventoryInputMode(false);
	}
}

void AGreenhousePlayerController::RegisterInputMappingContexts()
{
	if (!GetLocalPlayer())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* InputSubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!InputSubsystem)
	{
		return;
	}

	if (DefaultMappingContext)
	{
		InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
	}

	if (MouseLookMappingContext)
	{
		InputSubsystem->AddMappingContext(MouseLookMappingContext, 1);
	}
}

void AGreenhousePlayerController::ApplyInitialSpawnView()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	FRotator SpawnViewRotation = ControlledPawn->GetActorRotation();
	SpawnViewRotation.Pitch = 0.0f;
	SpawnViewRotation.Roll = 0.0f;
	SetControlRotation(SpawnViewRotation);
}

void AGreenhousePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent);
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AGreenhousePlayerController::ToggleInventory);
}

void AGreenhousePlayerController::ToggleInventory()
{
	if (!InventoryWidget)
	{
		return;
	}

	InventoryWidget->ToggleInventory();
	ApplyInventoryInputMode(InventoryWidget->IsInventoryOpen());
}

void AGreenhousePlayerController::ApplyInventoryInputMode(bool bInventoryOpen)
{
	bShowMouseCursor = bInventoryOpen;

	if (bInventoryOpen)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		if (InventoryWidget)
		{
			InputMode.SetWidgetToFocus(InventoryWidget->TakeWidget());
		}
		SetInputMode(InputMode);
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
	}
}
