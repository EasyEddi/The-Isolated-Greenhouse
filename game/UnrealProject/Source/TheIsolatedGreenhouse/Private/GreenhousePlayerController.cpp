#include "GreenhousePlayerController.h"

#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GreenhouseHeldItemActor.h"
#include "GreenhouseInventoryWidget.h"
#include "GreenhousePlantingPlotActor.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

AGreenhousePlayerController::AGreenhousePlayerController()
{
	PrimaryActorTick.bCanEverTick = true;

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
	ConfigurePlayerCollision();

	InventoryWidget = CreateWidget<UGreenhouseInventoryWidget>(this, UGreenhouseInventoryWidget::StaticClass());
	if (InventoryWidget)
	{
		InventoryWidget->AddToViewport();
		ApplyInventoryInputMode(false);
	}

	SpawnHeldItemActor();
	UpdateHeldItemActor();
}

void AGreenhousePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	UpdateHeldItemActor();
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

void AGreenhousePlayerController::ConfigurePlayerCollision()
{
	ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
	if (!ControlledCharacter)
	{
		return;
	}

	if (UCapsuleComponent* CapsuleComponent = ControlledCharacter->GetCapsuleComponent())
	{
		constexpr float DoorFriendlyRadiusCm = 24.0f;
		constexpr float FirstPersonHalfHeightCm = 90.0f;
		CapsuleComponent->SetCapsuleSize(DoorFriendlyRadiusCm, FirstPersonHalfHeightCm, true);
	}

	if (UCameraComponent* CameraComponent = ControlledCharacter->FindComponentByClass<UCameraComponent>())
	{
		constexpr float EyeHeightCm = 170.0f;
		constexpr float FirstPersonHalfHeightCm = 90.0f;
		CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, EyeHeightCm - FirstPersonHalfHeightCm));
	}

	if (UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement())
	{
		MovementComponent->MaxStepHeight = 90.0f;
		MovementComponent->PerchAdditionalHeight = 45.0f;
		MovementComponent->PerchRadiusThreshold = 0.0f;
		MovementComponent->SetWalkableFloorAngle(60.0f);
		MovementComponent->bUseFlatBaseForFloorChecks = false;
	}
}

void AGreenhousePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	check(InputComponent);
	InputComponent->BindKey(EKeys::E, IE_Pressed, this, &AGreenhousePlayerController::HandleInteractOrInventory);
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotOne);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotTwo);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotThree);
	InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotFour);
	InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &AGreenhousePlayerController::SelectHotbarSlotFive);
	InputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AGreenhousePlayerController::StartSprint);
	InputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &AGreenhousePlayerController::StopSprint);
	InputComponent->BindKey(EKeys::RightShift, IE_Pressed, this, &AGreenhousePlayerController::StartSprint);
	InputComponent->BindKey(EKeys::RightShift, IE_Released, this, &AGreenhousePlayerController::StopSprint);
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

void AGreenhousePlayerController::HandleInteractOrInventory()
{
	if (InventoryWidget && !InventoryWidget->IsInventoryOpen() && TryPlantSelectedLily())
	{
		return;
	}

	ToggleInventory();
}

bool AGreenhousePlayerController::TryPlantSelectedLily()
{
	if (!InventoryWidget || InventoryWidget->GetSelectedHotbarItem() != EGreenhouseInventoryItem::Lily)
	{
		return false;
	}

	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !GetWorld())
	{
		return false;
	}

	AGreenhousePlantingPlotActor* ClosestPlot = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();
	const FVector PlayerLocation = ControlledPawn->GetActorLocation();

	for (TActorIterator<AGreenhousePlantingPlotActor> PlotIt(GetWorld()); PlotIt; ++PlotIt)
	{
		AGreenhousePlantingPlotActor* Plot = *PlotIt;
		if (!Plot || !Plot->CanPlantAt(PlayerLocation))
		{
			continue;
		}

		const float DistanceSq = FVector::DistSquared2D(PlayerLocation, Plot->GetActorLocation());
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			ClosestPlot = Plot;
		}
	}

	if (!ClosestPlot || !InventoryWidget->ConsumeSelectedHotbarItem(EGreenhouseInventoryItem::Lily))
	{
		return false;
	}

	const bool bPlanted = ClosestPlot->PlantLily();
	if (!bPlanted)
	{
		InventoryWidget->AddItem(EGreenhouseInventoryItem::Lily);
	}

	UpdateHeldItemActor();
	return bPlanted;
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

void AGreenhousePlayerController::SpawnHeldItemActor()
{
	if (HeldItemActor || !GetWorld())
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	HeldItemActor = GetWorld()->SpawnActor<AGreenhouseHeldItemActor>(AGreenhouseHeldItemActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParameters);
	if (HeldItemActor)
	{
		HeldItemActor->SetActorHiddenInGame(true);
	}
}

USceneComponent* AGreenhousePlayerController::FindFirstPersonCameraComponent() const
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return nullptr;
	}

	return ControlledPawn->FindComponentByClass<UCameraComponent>();
}

void AGreenhousePlayerController::UpdateHeldItemActor()
{
	if (!InventoryWidget)
	{
		return;
	}

	SpawnHeldItemActor();
	if (!HeldItemActor || !PlayerCameraManager)
	{
		return;
	}

	const EGreenhouseInventoryItem SelectedItem = InventoryWidget->GetSelectedHotbarItem();
	if (SelectedItem != LastDisplayedHeldItem)
	{
		HeldItemActor->SetHeldItem(SelectedItem);
		LastDisplayedHeldItem = SelectedItem;
	}

	if (USceneComponent* CameraComponent = FindFirstPersonCameraComponent())
	{
		if (HeldItemActor->GetRootComponent()->GetAttachParent() != CameraComponent)
		{
			HeldItemActor->AttachToComponent(CameraComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}

		HeldItemActor->SetActorRelativeLocation(FVector(58.0f, 25.0f, -32.0f));
		HeldItemActor->SetActorRelativeRotation(FRotator(-8.0f, 18.0f, 8.0f));
		return;
	}

	const FRotator CameraRotation = PlayerCameraManager->GetCameraRotation();
	const FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
	const FVector HandOffset = CameraRotation.RotateVector(FVector(58.0f, 25.0f, -32.0f));
	const FRotator ItemRotation = CameraRotation + FRotator(-8.0f, 18.0f, 8.0f);
	HeldItemActor->SetActorLocationAndRotation(CameraLocation + HandOffset, ItemRotation);
}

void AGreenhousePlayerController::SelectHotbarSlot(int32 SlotIndex)
{
	if (!InventoryWidget)
	{
		return;
	}

	InventoryWidget->SelectHotbarSlot(SlotIndex);
	UpdateHeldItemActor();
}

void AGreenhousePlayerController::SelectHotbarSlotOne()
{
	SelectHotbarSlot(0);
}

void AGreenhousePlayerController::SelectHotbarSlotTwo()
{
	SelectHotbarSlot(1);
}

void AGreenhousePlayerController::SelectHotbarSlotThree()
{
	SelectHotbarSlot(2);
}

void AGreenhousePlayerController::SelectHotbarSlotFour()
{
	SelectHotbarSlot(3);
}

void AGreenhousePlayerController::SelectHotbarSlotFive()
{
	SelectHotbarSlot(4);
}

void AGreenhousePlayerController::StartSprint()
{
	bIsSprinting = true;
	ApplySprintSpeed();
}

void AGreenhousePlayerController::StopSprint()
{
	bIsSprinting = false;
	ApplySprintSpeed();
}

void AGreenhousePlayerController::ApplySprintSpeed()
{
	ACharacter* ControlledCharacter = Cast<ACharacter>(GetPawn());
	if (!ControlledCharacter)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = ControlledCharacter->GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	if (DefaultWalkSpeed <= 0.0f)
	{
		DefaultWalkSpeed = MovementComponent->MaxWalkSpeed;
	}

	MovementComponent->MaxWalkSpeed = bIsSprinting ? DefaultWalkSpeed * SprintSpeedMultiplier : DefaultWalkSpeed;
}
