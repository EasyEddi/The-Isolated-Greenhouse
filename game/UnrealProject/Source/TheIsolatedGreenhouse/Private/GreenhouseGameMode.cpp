#include "GreenhouseGameMode.h"

#include "GreenhousePlantingPlotActor.h"
#include "GreenhousePlayerController.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "UObject/ConstructorHelpers.h"

AGreenhouseGameMode::AGreenhouseGameMode()
{
	PlayerControllerClass = AGreenhousePlayerController::StaticClass();

	static ConstructorHelpers::FClassFinder<APawn> FirstPersonPawnClass(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	if (FirstPersonPawnClass.Succeeded())
	{
		DefaultPawnClass = FirstPersonPawnClass.Class;
	}
}

void AGreenhouseGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!GetWorld())
	{
		return;
	}

	TActorIterator<AGreenhousePlantingPlotActor> ExistingPlotIt(GetWorld());
	if (ExistingPlotIt)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AGreenhousePlantingPlotActor* Plot = GetWorld()->SpawnActor<AGreenhousePlantingPlotActor>(
		AGreenhousePlantingPlotActor::StaticClass(),
		FVector(350.0f, 1120.0f, 2.0f),
		FRotator::ZeroRotator,
		SpawnParameters);

	if (Plot)
	{
#if WITH_EDITOR
		Plot->SetActorLabel(TEXT("PlantingPlot_GreenhouseZone_01"));
#endif
	}
}
