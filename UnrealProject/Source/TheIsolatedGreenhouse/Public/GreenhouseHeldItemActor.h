#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GreenhouseInventoryWidget.h"
#include "GreenhouseHeldItemActor.generated.h"

class UMaterialInstanceDynamic;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class THEISOLATEDGREENHOUSE_API AGreenhouseHeldItemActor : public AActor
{
	GENERATED_BODY()

public:
	AGreenhouseHeldItemActor();

	void SetHeldItem(EGreenhouseInventoryItem Item);

private:
	UPROPERTY()
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> LilyParts;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> WateringCanParts;

	EGreenhouseInventoryItem CurrentItem = EGreenhouseInventoryItem::None;

	UStaticMeshComponent* CreatePart(FName Name, UStaticMesh* Mesh, const FLinearColor& Color);
	void SetPartVisibility(const TArray<TObjectPtr<UStaticMeshComponent>>& Parts, bool bVisible);
};
