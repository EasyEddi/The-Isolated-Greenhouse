#include "GreenhouseHeldItemActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
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
	UStaticMesh* CylinderMesh = LoadFirstAvailableMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	LilyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LilyMesh"));
	LilyMeshComponent->SetupAttachment(SceneRoot);
	LilyMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LilyMeshComponent->SetCastShadow(false);
	LilyMeshComponent->SetStaticMesh(LilyMesh);
	LilyMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 6.0f));
	LilyMeshComponent->SetRelativeRotation(FRotator(0.0f, 170.0f, 0.0f));
	LilyMeshComponent->SetRelativeScale3D(FVector(0.2f));

	WateringCanMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WateringCanMesh"));
	WateringCanMeshComponent->SetupAttachment(SceneRoot);
	WateringCanMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WateringCanMeshComponent->SetCastShadow(false);
	WateringCanMeshComponent->SetStaticMesh(WateringCanMesh);
	WateringCanMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, -12.0f));
	WateringCanMeshComponent->SetRelativeRotation(FRotator(0.0f, 145.0f, 0.0f));
	WateringCanMeshComponent->SetRelativeScale3D(FVector(0.32f));

	WaterStreamMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterStreamMesh"));
	WaterStreamMeshComponent->SetupAttachment(SceneRoot);
	WaterStreamMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WaterStreamMeshComponent->SetCastShadow(false);
	WaterStreamMeshComponent->SetStaticMesh(CylinderMesh);
	WaterStreamMeshComponent->SetRelativeScale3D(FVector(0.018f, 0.018f, 0.62f));
	WaterStreamMeshComponent->SetVisibility(false, true);
	if (UMaterialInstanceDynamic* WaterMaterial = WaterStreamMeshComponent->CreateAndSetMaterialInstanceDynamic(0))
	{
		WaterMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.20f, 0.58f, 1.0f, 0.78f));
		WaterMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.20f, 0.58f, 1.0f, 0.78f));
		WaterMaterial->SetScalarParameterValue(TEXT("Opacity"), 0.68f);
	}

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

	if (Item != EGreenhouseInventoryItem::WateringCan)
	{
		SetWaterEffect(EGreenhouseHeldWaterEffect::None);
	}

	SetActorHiddenInGame(Item == EGreenhouseInventoryItem::None);
}

void AGreenhouseHeldItemActor::SetWaterEffect(EGreenhouseHeldWaterEffect Effect)
{
	CurrentWaterEffect = Effect;
	if (!WaterStreamMeshComponent)
	{
		return;
	}

	const bool bShowWater = CurrentItem == EGreenhouseInventoryItem::WateringCan && Effect != EGreenhouseHeldWaterEffect::None;
	WaterStreamMeshComponent->SetVisibility(bShowWater, true);
	if (!bShowWater)
	{
		return;
	}

	if (Effect == EGreenhouseHeldWaterEffect::Filling)
	{
		WaterStreamMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 78.0f));
		WaterStreamMeshComponent->SetRelativeRotation(FRotator::ZeroRotator);
		WaterStreamMeshComponent->SetRelativeScale3D(FVector(0.016f, 0.016f, 0.72f));
	}
	else
	{
		const FVector StreamStart(52.0f, -36.0f, 18.0f);
		const FVector StreamEnd(78.0f, -46.0f, -58.0f);
		const FVector StreamVector = StreamEnd - StreamStart;
		const float StreamLength = StreamVector.Size();
		const FVector StreamDirection = StreamLength > 0.0f ? StreamVector / StreamLength : FVector::DownVector;

		WaterStreamMeshComponent->SetRelativeLocation(StreamStart + StreamVector * 0.5f);
		WaterStreamMeshComponent->SetRelativeRotation(FQuat::FindBetweenNormals(FVector::UpVector, StreamDirection).Rotator());
		WaterStreamMeshComponent->SetRelativeScale3D(FVector(0.018f, 0.018f, StreamLength / 100.0f));
	}
}
