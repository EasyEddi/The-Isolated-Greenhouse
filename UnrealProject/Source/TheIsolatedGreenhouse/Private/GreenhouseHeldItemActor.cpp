#include "GreenhouseHeldItemActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AGreenhouseHeldItemActor::AGreenhouseHeldItemActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshFinder(TEXT("/Engine/BasicShapes/Sphere"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMeshFinder(TEXT("/Engine/BasicShapes/Cylinder"));

	UStaticMesh* CubeMesh = CubeMeshFinder.Object;
	UStaticMesh* SphereMesh = SphereMeshFinder.Object;
	UStaticMesh* CylinderMesh = CylinderMeshFinder.Object;

	UStaticMeshComponent* LilyStem = CreatePart(TEXT("LilyStem"), CylinderMesh, FLinearColor(0.08f, 0.45f, 0.17f, 1.0f));
	LilyStem->SetRelativeLocation(FVector(0.0f, 0.0f, 13.0f));
	LilyStem->SetRelativeScale3D(FVector(0.035f, 0.035f, 0.36f));
	LilyParts.Add(LilyStem);

	UStaticMeshComponent* LilyLeaf = CreatePart(TEXT("LilyLeaf"), CubeMesh, FLinearColor(0.04f, 0.62f, 0.30f, 1.0f));
	LilyLeaf->SetRelativeLocation(FVector(0.0f, -7.0f, 8.0f));
	LilyLeaf->SetRelativeRotation(FRotator(0.0f, 0.0f, 28.0f));
	LilyLeaf->SetRelativeScale3D(FVector(0.10f, 0.035f, 0.018f));
	LilyParts.Add(LilyLeaf);

	UStaticMeshComponent* LilyBloom = CreatePart(TEXT("LilyBloom"), SphereMesh, FLinearColor(0.95f, 0.88f, 0.94f, 1.0f));
	LilyBloom->SetRelativeLocation(FVector(0.0f, 0.0f, 34.0f));
	LilyBloom->SetRelativeScale3D(FVector(0.13f, 0.13f, 0.10f));
	LilyParts.Add(LilyBloom);

	UStaticMeshComponent* CanBody = CreatePart(TEXT("WateringCanBody"), CubeMesh, FLinearColor(0.18f, 0.45f, 0.62f, 1.0f));
	CanBody->SetRelativeLocation(FVector(0.0f, 0.0f, 12.0f));
	CanBody->SetRelativeScale3D(FVector(0.20f, 0.14f, 0.16f));
	WateringCanParts.Add(CanBody);

	UStaticMeshComponent* CanTop = CreatePart(TEXT("WateringCanTop"), CylinderMesh, FLinearColor(0.14f, 0.36f, 0.50f, 1.0f));
	CanTop->SetRelativeLocation(FVector(0.0f, 0.0f, 28.0f));
	CanTop->SetRelativeScale3D(FVector(0.08f, 0.08f, 0.06f));
	WateringCanParts.Add(CanTop);

	UStaticMeshComponent* CanSpout = CreatePart(TEXT("WateringCanSpout"), CubeMesh, FLinearColor(0.16f, 0.42f, 0.58f, 1.0f));
	CanSpout->SetRelativeLocation(FVector(18.0f, 0.0f, 19.0f));
	CanSpout->SetRelativeRotation(FRotator(0.0f, 0.0f, 14.0f));
	CanSpout->SetRelativeScale3D(FVector(0.22f, 0.035f, 0.035f));
	WateringCanParts.Add(CanSpout);

	UStaticMeshComponent* CanHandle = CreatePart(TEXT("WateringCanHandle"), CubeMesh, FLinearColor(0.12f, 0.30f, 0.42f, 1.0f));
	CanHandle->SetRelativeLocation(FVector(-16.0f, 0.0f, 18.0f));
	CanHandle->SetRelativeScale3D(FVector(0.035f, 0.04f, 0.22f));
	WateringCanParts.Add(CanHandle);

	SetHeldItem(EGreenhouseInventoryItem::None);
}

UStaticMeshComponent* AGreenhouseHeldItemActor::CreatePart(FName Name, UStaticMesh* Mesh, const FLinearColor& Color)
{
	UStaticMeshComponent* Part = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Part->SetupAttachment(SceneRoot);
	Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Part->SetCastShadow(false);
	Part->SetStaticMesh(Mesh);

	UMaterialInstanceDynamic* DynamicMaterial = Part->CreateAndSetMaterialInstanceDynamic(0);
	if (DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(TEXT("Color"), Color);
		DynamicMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
	}

	return Part;
}

void AGreenhouseHeldItemActor::SetHeldItem(EGreenhouseInventoryItem Item)
{
	if (CurrentItem == Item)
	{
		return;
	}

	CurrentItem = Item;
	SetPartVisibility(LilyParts, Item == EGreenhouseInventoryItem::Lily);
	SetPartVisibility(WateringCanParts, Item == EGreenhouseInventoryItem::WateringCan);
	SetActorHiddenInGame(Item == EGreenhouseInventoryItem::None);
}

void AGreenhouseHeldItemActor::SetPartVisibility(const TArray<TObjectPtr<UStaticMeshComponent>>& Parts, bool bVisible)
{
	for (UStaticMeshComponent* Part : Parts)
	{
		if (Part)
		{
			Part->SetVisibility(bVisible, true);
		}
	}
}
