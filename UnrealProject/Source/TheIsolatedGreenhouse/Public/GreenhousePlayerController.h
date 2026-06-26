#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GreenhousePlayerController.generated.h"

class UGreenhouseInventoryWidget;

UCLASS()
class THEISOLATEDGREENHOUSE_API AGreenhousePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	UPROPERTY()
	TObjectPtr<UGreenhouseInventoryWidget> InventoryWidget;

	void ToggleInventory();
	void ApplyInventoryInputMode(bool bInventoryOpen);
};
