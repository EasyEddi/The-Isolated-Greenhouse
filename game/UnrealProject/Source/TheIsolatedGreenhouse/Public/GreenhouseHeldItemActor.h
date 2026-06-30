#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreenhouseInventoryWidget.h"
#include "GreenhouseHeldItemActor.generated.h"

class UStaticMeshComponent;

enum class EGreenhouseHeldWaterEffect : uint8
{
	None,
	Pouring,
	Filling
};

UCLASS()
class THEISOLATEDGREENHOUSE_API AGreenhouseHeldItemActor : public AActor
{
	GENERATED_BODY()

public:
	AGreenhouseHeldItemActor();

	void SetHeldItem(EGreenhouseInventoryItem Item);
	void SetWaterEffect(EGreenhouseHeldWaterEffect Effect);

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> LilyMeshComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WateringCanMeshComponent;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> WaterStreamMeshComponent;

	EGreenhouseInventoryItem CurrentItem = EGreenhouseInventoryItem::None;
	EGreenhouseHeldWaterEffect CurrentWaterEffect = EGreenhouseHeldWaterEffect::None;
};
