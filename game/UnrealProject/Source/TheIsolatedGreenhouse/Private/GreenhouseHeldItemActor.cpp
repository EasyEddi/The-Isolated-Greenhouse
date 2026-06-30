#include "GreenhouseHeldItemActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
UStaticMesh* LoadFirstAvailableMesh(const TCHAR* PrimaryPath, const TCHAR* FallbackPath = nullptr)
{
	if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, PrimaryPath))
	{
		return Mesh;
	}

	return FallbackPath ? LoadObject<UStaticMesh>(nullptr, FallbackPath) : nullptr;
}

void ConfigureHeldMesh(UStaticMeshComponent* MeshComponent, UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, float Scale)
{
	if (!MeshComponent)
	{
		return;
	}

	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetCastShadow(false);
	MeshComponent->SetStaticMesh(Mesh);
	MeshComponent->SetRelativeLocation(Location);
	MeshComponent->SetRelativeRotation(Rotation);
	MeshComponent->SetRelativeScale3D(FVector(Scale));
}
}

AGreenhouseHeldItemActor::AGreenhouseHeldItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	UStaticMesh* LilyMesh = LoadFirstAvailableMesh(TEXT("/Game/models/Plants/Lily/lily.lily"));
	UStaticMesh* WateringCanMesh = LoadFirstAvailableMesh(
		TEXT("/Game/models/equipment/Watering_Can/watering_can.watering_can"),
		TEXT("/Game/models/furniture/Watering_Can/watering_can.watering_can"));
	UStaticMesh* EmptyPotMesh = LoadFirstAvailableMesh(
		TEXT("/Game/models/equipment/pots/ornament_plants/empty/ornament_plants_pot_empty.ornament_plants_pot_empty"),
		TEXT("/Game/models/equipment/pots/ornament plants/empty/ornament_plants_pot_empty.ornament_plants_pot_empty"));
	UStaticMesh* SoilBagMesh = LoadFirstAvailableMesh(
		TEXT("/Game/models/equipment/soil/ornament_plants/ornament_plants_soil.ornament_plants_soil"),
		TEXT("/Game/models/equipment/soil/ornament plants/ornament_plants_soil.ornament_plants_soil"));
	UStaticMesh* FertilizerBagMesh = LoadFirstAvailableMesh(
		TEXT("/Game/models/equipment/fertilizer/ornament_plants/ornament_plants_fertilizer.ornament_plants_fertilizer"));

	LilyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LilyMesh"));
	LilyMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(LilyMeshComponent, LilyMesh, FVector(0.0f, 0.0f, 6.0f), FRotator(0.0f, 170.0f, 0.0f), 0.2f);

	WateringCanMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WateringCanMesh"));
	WateringCanMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(WateringCanMeshComponent, WateringCanMesh, FVector(0.0f, 0.0f, -12.0f), FRotator(0.0f, 145.0f, 0.0f), 0.32f);

	EmptyPotMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("EmptyPotMesh"));
	EmptyPotMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(EmptyPotMeshComponent, EmptyPotMesh, FVector(0.0f, 0.0f, -12.0f), FRotator(0.0f, 150.0f, 0.0f), 0.30f);

	SoilBagMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SoilBagMesh"));
	SoilBagMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(SoilBagMeshComponent, SoilBagMesh, FVector(0.0f, 0.0f, -14.0f), FRotator(0.0f, 148.0f, 0.0f), 0.25f);

	FertilizerBagMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FertilizerBagMesh"));
	FertilizerBagMeshComponent->SetupAttachment(SceneRoot);
	ConfigureHeldMesh(FertilizerBagMeshComponent, FertilizerBagMesh, FVector(0.0f, 0.0f, -14.0f), FRotator(0.0f, 148.0f, 0.0f), 0.25f);

	SetHeldItem(EGreenhouseInventoryItem::None);
}

void AGreenhouseHeldItemActor::SetHeldItem(EGreenhouseInventoryItem Item)
{
	CurrentItem = Item;
	if (LilyMeshComponent)
	{
		LilyMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::Lily, true);
	}

	if (WateringCanMeshComponent)
	{
		WateringCanMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::WateringCan, true);
	}

	if (EmptyPotMeshComponent)
	{
		EmptyPotMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::EmptyPot, true);
	}

	if (SoilBagMeshComponent)
	{
		SoilBagMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::SoilBag, true);
	}

	if (FertilizerBagMeshComponent)
	{
		FertilizerBagMeshComponent->SetVisibility(Item == EGreenhouseInventoryItem::FertilizerBag, true);
	}

	SetActorHiddenInGame(Item == EGreenhouseInventoryItem::None);
}
