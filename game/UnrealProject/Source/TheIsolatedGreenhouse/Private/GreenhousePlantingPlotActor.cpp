#include "GreenhousePlantingPlotActor.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AGreenhousePlantingPlotActor::AGreenhousePlantingPlotActor()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* LilyMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/models/Plants/Lily/lily.lily"));

	PlotMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlotMesh"));
	SetRootComponent(PlotMeshComponent);
	PlotMeshComponent->SetStaticMesh(CubeMeshFinder.Object);
	PlotMeshComponent->SetRelativeScale3D(FVector(1.05f, 1.05f, 0.04f));
	PlotMeshComponent->SetCollisionProfileName(TEXT("BlockAll"));

	if (UMaterialInstanceDynamic* PlotMaterial = PlotMeshComponent->CreateAndSetMaterialInstanceDynamic(0))
	{
		PlotMaterial->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.16f, 0.24f, 0.18f, 1.0f));
		PlotMaterial->SetScalarParameterValue(TEXT("Roughness"), 0.92f);
	}

	LilyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrowingLilyMesh"));
	LilyMeshComponent->SetupAttachment(PlotMeshComponent);
	LilyMeshComponent->SetStaticMesh(LilyMesh);
	LilyMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LilyMeshComponent->SetCastShadow(true);
	LilyMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 8.0f));
	LilyMeshComponent->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f));
	LilyMeshComponent->SetRelativeScale3D(FVector(0.02f));
	LilyMeshComponent->SetVisibility(false, true);
}

void AGreenhousePlantingPlotActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsGrowing || !LilyMeshComponent)
	{
		return;
	}

	GrowthSeconds = FMath::Min(GrowthSeconds + DeltaSeconds, GrowthDuration);
	const float Alpha = GrowthSeconds / GrowthDuration;
	const float Scale = FMath::Lerp(0.02f, 0.24f, Alpha);
	LilyMeshComponent->SetRelativeScale3D(FVector(Scale));

	if (GrowthSeconds >= GrowthDuration)
	{
		bIsGrowing = false;
	}
}

bool AGreenhousePlantingPlotActor::CanPlantAt(const FVector& PlayerLocation) const
{
	if (bHasLily)
	{
		return false;
	}

	const FVector PlotLocation = GetActorLocation();
	const float Distance2D = FVector::Dist2D(PlayerLocation, PlotLocation);
	return Distance2D <= InteractionRadius;
}

bool AGreenhousePlantingPlotActor::PlantLily()
{
	if (bHasLily || !LilyMeshComponent)
	{
		return false;
	}

	bHasLily = true;
	bIsGrowing = true;
	GrowthSeconds = 0.0f;
	LilyMeshComponent->SetRelativeScale3D(FVector(0.02f));
	LilyMeshComponent->SetVisibility(true, true);
	return true;
}
