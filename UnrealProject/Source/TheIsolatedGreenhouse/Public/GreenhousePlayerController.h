#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GreenhousePlayerController.generated.h"

class UGreenhouseInventoryWidget;
class UInputMappingContext;

UCLASS()
class THEISOLATEDGREENHOUSE_API AGreenhousePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGreenhousePlayerController();

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	TObjectPtr<UGreenhouseInventoryWidget> InventoryWidget;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> MouseLookMappingContext;

	void ToggleInventory();
	void ApplyInventoryInputMode(bool bInventoryOpen);
	void RegisterInputMappingContexts();
	void ApplyInitialSpawnView();
};
