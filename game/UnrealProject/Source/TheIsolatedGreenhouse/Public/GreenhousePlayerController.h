#pragma once

#include "CoreMinimal.h"
#include "GreenhouseInventoryWidget.h"
#include "GameFramework/PlayerController.h"
#include "GreenhousePlayerController.generated.h"

class AGreenhouseHeldItemActor;
class AGreenhousePlantingPlotActor;
class UGreenhouseInventoryWidget;
class UInputMappingContext;
class USceneComponent;

UCLASS()
class THEISOLATEDGREENHOUSE_API AGreenhousePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGreenhousePlayerController();

	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

private:
	static constexpr float SprintSpeedMultiplier = 1.5f;
	static constexpr float InteractionTraceDistance = 520.0f;
	static constexpr float MaxWateringCanLiters = 10.0f;
	static constexpr float WateringCanFillDuration = 7.0f;
	static constexpr float WateringLitersPerSecond = 1.15f;

	UPROPERTY()
	TObjectPtr<UGreenhouseInventoryWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	UPROPERTY()
	TObjectPtr<AGreenhouseHeldItemActor> HeldItemActor;

	EGreenhouseInventoryItem LastDisplayedHeldItem = EGreenhouseInventoryItem::None;
	float DefaultWalkSpeed = 0.0f;
	float WateringCanLiters = 0.0f;
	float WateringCanFillSeconds = 0.0f;
	float WateringCanFillStartLiters = 0.0f;
	FTransform WateringCanFillingTransform = FTransform::Identity;
	TWeakObjectPtr<AGreenhousePlantingPlotActor> CurrentWateringPlot;
	bool bIsSprinting = false;
	bool bIsFillingWateringCan = false;
	bool bIsPouringWater = false;

	void ToggleInventory();
	void HandleInteractOrInventory();
	bool TryPlantSelectedLily();
	void HandleWateringCanPressed();
	void HandleWateringCanReleased();
	void ApplyInventoryInputMode(bool bInventoryOpen);
	void RegisterInputMappingContexts();
	void ApplyInitialSpawnView();
	void ConfigurePlayerCollision();
	void SpawnHeldItemActor();
	USceneComponent* FindFirstPersonCameraComponent() const;
	void UpdateHeldItemActor();
	void UpdateWateringCanState(float DeltaTime);
	bool HasSelectedWateringCan() const;
	bool TraceForInteraction(FHitResult& OutHit) const;
	bool IsFaucetHit(const FHitResult& Hit) const;
	void StartFillingWateringCan(const FHitResult& FaucetHit);
	void FinishFillingWateringCan();
	void StartPouringWater(const FHitResult& InteractionHit);
	void StopPouringWater();
	AGreenhousePlantingPlotActor* FindWateringPlot(const FHitResult& InteractionHit) const;
	FTransform BuildFillingCanTransform(const FHitResult& FaucetHit) const;
	void SelectHotbarSlot(int32 SlotIndex);
	void SelectHotbarSlotOne();
	void SelectHotbarSlotTwo();
	void SelectHotbarSlotThree();
	void SelectHotbarSlotFour();
	void SelectHotbarSlotFive();
	void StartSprint();
	void StopSprint();
	void ApplySprintSpeed();
};
