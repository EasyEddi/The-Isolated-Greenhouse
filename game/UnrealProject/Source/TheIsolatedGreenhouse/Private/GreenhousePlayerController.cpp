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
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"
#include "Engine/StaticMesh.h"
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

	UpdateWateringCanState(DeltaTime);
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
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &AGreenhousePlayerController::HandleWateringCanPressed);
	InputComponent->BindKey(EKeys::RightMouseButton, IE_Released, this, &AGreenhousePlayerController::HandleWateringCanReleased);
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

void AGreenhousePlayerController::HandleWateringCanPressed()
{
	if (InventoryWidget && InventoryWidget->IsInventoryOpen())
	{
		return;
	}

	if (!HasSelectedWateringCan() || bIsFillingWateringCan)
	{
		return;
	}

	FHitResult InteractionHit;
	TraceForInteraction(InteractionHit);
	if (InteractionHit.bBlockingHit && IsFaucetHit(InteractionHit) && WateringCanLiters < MaxWateringCanLiters - 0.05f)
	{
		StartFillingWateringCan(InteractionHit);
		return;
	}

	StartPouringWater(InteractionHit);
}

void AGreenhousePlayerController::HandleWateringCanReleased()
{
	StopPouringWater();
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

void AGreenhousePlayerController::UpdateWateringCanState(float DeltaTime)
{
	if (bIsFillingWateringCan)
	{
		WateringCanFillSeconds = FMath::Min(WateringCanFillSeconds + DeltaTime, WateringCanFillDuration);
		const float FillAlpha = WateringCanFillDuration > 0.0f ? WateringCanFillSeconds / WateringCanFillDuration : 1.0f;
		WateringCanLiters = FMath::Lerp(WateringCanFillStartLiters, MaxWateringCanLiters, FillAlpha);

		if (FillAlpha >= 1.0f)
		{
			FinishFillingWateringCan();
		}
		return;
	}

	if (!bIsPouringWater)
	{
		return;
	}

	WateringCanLiters = FMath::Max(0.0f, WateringCanLiters - DeltaTime * WateringLitersPerSecond);
	if (CurrentWateringPlot.IsValid())
	{
		CurrentWateringPlot->WaterPlant(DeltaTime);
	}

	if (WateringCanLiters <= 0.0f)
	{
		StopPouringWater();
	}
}

bool AGreenhousePlayerController::HasSelectedWateringCan() const
{
	return InventoryWidget && InventoryWidget->GetSelectedHotbarItem() == EGreenhouseInventoryItem::WateringCan;
}

bool AGreenhousePlayerController::TraceForInteraction(FHitResult& OutHit) const
{
	if (!PlayerCameraManager || !GetWorld())
	{
		return false;
	}

	const FVector TraceStart = PlayerCameraManager->GetCameraLocation();
	const FVector TraceEnd = TraceStart + PlayerCameraManager->GetCameraRotation().Vector() * InteractionTraceDistance;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(GreenhouseWateringInteraction), false);
	if (const APawn* ControlledPawn = GetPawn())
	{
		QueryParams.AddIgnoredActor(ControlledPawn);
	}
	if (HeldItemActor)
	{
		QueryParams.AddIgnoredActor(HeldItemActor);
	}

	return GetWorld()->LineTraceSingleByChannel(OutHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
}

bool AGreenhousePlayerController::IsFaucetHit(const FHitResult& Hit) const
{
	const AActor* HitActor = Hit.GetActor();
	const UStaticMeshComponent* MeshComponent = Cast<UStaticMeshComponent>(Hit.GetComponent());
	const UStaticMesh* Mesh = MeshComponent ? MeshComponent->GetStaticMesh() : nullptr;

	const FString ActorName = HitActor ? HitActor->GetName() : FString();
	const FString ComponentName = MeshComponent ? MeshComponent->GetName() : FString();
	const FString MeshPath = Mesh ? Mesh->GetPathName() : FString();

	return ActorName.Contains(TEXT("Faucet"), ESearchCase::IgnoreCase)
		|| ActorName.Contains(TEXT("WaterTap"), ESearchCase::IgnoreCase)
		|| ActorName.Contains(TEXT("Wasserhahn"), ESearchCase::IgnoreCase)
		|| ComponentName.Contains(TEXT("Faucet"), ESearchCase::IgnoreCase)
		|| MeshPath.Contains(TEXT("Garden_Faucet"), ESearchCase::IgnoreCase)
		|| MeshPath.Contains(TEXT("garden_faucet"), ESearchCase::IgnoreCase)
		|| MeshPath.Contains(TEXT("WaterTap"), ESearchCase::IgnoreCase);
}

void AGreenhousePlayerController::StartFillingWateringCan(const FHitResult& FaucetHit)
{
	StopPouringWater();
	bIsFillingWateringCan = true;
	WateringCanFillSeconds = 0.0f;
	WateringCanFillStartLiters = WateringCanLiters;
	WateringCanFillingTransform = BuildFillingCanTransform(FaucetHit);

	if (HeldItemActor)
	{
		HeldItemActor->SetHeldItem(EGreenhouseInventoryItem::WateringCan);
		HeldItemActor->SetWaterEffect(EGreenhouseHeldWaterEffect::Filling);
	}
}

void AGreenhousePlayerController::FinishFillingWateringCan()
{
	bIsFillingWateringCan = false;
	WateringCanFillSeconds = 0.0f;
	WateringCanFillStartLiters = MaxWateringCanLiters;
	WateringCanLiters = MaxWateringCanLiters;
	if (HeldItemActor)
	{
		HeldItemActor->SetWaterEffect(EGreenhouseHeldWaterEffect::None);
	}
}

void AGreenhousePlayerController::StartPouringWater(const FHitResult& InteractionHit)
{
	if (WateringCanLiters <= 0.0f)
	{
		return;
	}

	bIsPouringWater = true;
	CurrentWateringPlot = FindWateringPlot(InteractionHit);
	if (CurrentWateringPlot.IsValid())
	{
		CurrentWateringPlot->WaterPlant(0.0f);
	}

	if (HeldItemActor)
	{
		HeldItemActor->SetWaterEffect(EGreenhouseHeldWaterEffect::Pouring);
	}
}

void AGreenhousePlayerController::StopPouringWater()
{
	bIsPouringWater = false;
	CurrentWateringPlot.Reset();
	if (HeldItemActor && !bIsFillingWateringCan)
	{
		HeldItemActor->SetWaterEffect(EGreenhouseHeldWaterEffect::None);
	}
}

AGreenhousePlantingPlotActor* AGreenhousePlayerController::FindWateringPlot(const FHitResult& InteractionHit) const
{
	const APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn || !GetWorld())
	{
		return nullptr;
	}

	const FVector PlayerLocation = ControlledPawn->GetActorLocation();
	if (AGreenhousePlantingPlotActor* HitPlot = Cast<AGreenhousePlantingPlotActor>(InteractionHit.GetActor()))
	{
		if (HitPlot->CanWaterAt(PlayerLocation))
		{
			return HitPlot;
		}
	}

	AGreenhousePlantingPlotActor* ClosestPlot = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();
	for (TActorIterator<AGreenhousePlantingPlotActor> PlotIt(GetWorld()); PlotIt; ++PlotIt)
	{
		AGreenhousePlantingPlotActor* Plot = *PlotIt;
		if (!Plot || !Plot->CanWaterAt(PlayerLocation))
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

	return ClosestPlot;
}

FTransform AGreenhousePlayerController::BuildFillingCanTransform(const FHitResult& FaucetHit) const
{
	const FVector ClickLocation = FaucetHit.bBlockingHit ? FaucetHit.ImpactPoint : FVector::ZeroVector;
	FVector CanLocation = ClickLocation + FVector(0.0f, 0.0f, -72.0f);

	if (const UPrimitiveComponent* HitComponent = FaucetHit.GetComponent())
	{
		const FBoxSphereBounds Bounds = HitComponent->Bounds;
		CanLocation.Z = FMath::Min(CanLocation.Z, Bounds.Origin.Z - Bounds.BoxExtent.Z + 24.0f);
	}

	const float Yaw = PlayerCameraManager ? PlayerCameraManager->GetCameraRotation().Yaw + 180.0f : 0.0f;
	return FTransform(FRotator(0.0f, Yaw, 0.0f), CanLocation, FVector(1.0f));
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

	if (bIsFillingWateringCan)
	{
		if (LastDisplayedHeldItem != EGreenhouseInventoryItem::WateringCan)
		{
			HeldItemActor->SetHeldItem(EGreenhouseInventoryItem::WateringCan);
			LastDisplayedHeldItem = EGreenhouseInventoryItem::WateringCan;
		}

		HeldItemActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		HeldItemActor->SetActorTransform(WateringCanFillingTransform);
		HeldItemActor->SetWaterEffect(EGreenhouseHeldWaterEffect::Filling);
		return;
	}

	const EGreenhouseInventoryItem SelectedItem = InventoryWidget->GetSelectedHotbarItem();
	if (SelectedItem != LastDisplayedHeldItem)
	{
		HeldItemActor->SetHeldItem(SelectedItem);
		LastDisplayedHeldItem = SelectedItem;
	}
	HeldItemActor->SetWaterEffect(bIsPouringWater ? EGreenhouseHeldWaterEffect::Pouring : EGreenhouseHeldWaterEffect::None);

	const FVector HeldLocation = bIsPouringWater
		? FVector(62.0f, 30.0f, -35.0f)
		: FVector(58.0f, 25.0f, -32.0f);
	const FRotator HeldRotation = bIsPouringWater
		? FRotator(28.0f, 22.0f, 24.0f)
		: FRotator(-8.0f, 18.0f, 8.0f);

	if (USceneComponent* CameraComponent = FindFirstPersonCameraComponent())
	{
		if (HeldItemActor->GetRootComponent()->GetAttachParent() != CameraComponent)
		{
			HeldItemActor->AttachToComponent(CameraComponent, FAttachmentTransformRules::KeepRelativeTransform);
		}

		HeldItemActor->SetActorRelativeLocation(HeldLocation);
		HeldItemActor->SetActorRelativeRotation(HeldRotation);
		return;
	}

	const FRotator CameraRotation = PlayerCameraManager->GetCameraRotation();
	const FVector CameraLocation = PlayerCameraManager->GetCameraLocation();
	const FVector HandOffset = CameraRotation.RotateVector(HeldLocation);
	const FRotator ItemRotation = CameraRotation + HeldRotation;
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
