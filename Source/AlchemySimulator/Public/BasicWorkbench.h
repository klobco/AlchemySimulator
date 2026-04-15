// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicInteractableStationObject.h"
#include "BasicWorkbench.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API ABasicWorkbench : public ABasicInteractableStationObject
{
	GENERATED_BODY()

public:

	ABasicWorkbench();

	UPROPERTY(EditAnywhere, Category = "components")
	UStaticMeshComponent* KnifeStand;

	UPROPERTY(EditAnywhere, Category = "components")
	UStaticMeshComponent* HerbStand;

	UPROPERTY(EditAnywhere, Category = "herbs")
	TMap<int32, class ABasePlant* > Herbs;

	UPROPERTY(EditAnywhere, Category = "herbs")
	ABasePlant* HerbOnTable;

	UFUNCTION(BlueprintCallable)
	virtual void AddTool(ABaseTool* tool) override;

	UFUNCTION(BlueprintCallable)
	virtual void RemoveTool(ABaseTool* tool) override;

	UFUNCTION(BlueprintCallable)
	void MovePlantToManipulation(ABasePlant* Herb);
	
private:
	UFUNCTION()
	void HandleHerbsInvenotyChange();

	void CreateHerb(FInventorySlot InSlot, int32 indexToAdd);

	void RemoveHerb(int32 indexToRemove);

	void MoveHerb(int32 from, int32 to);

protected:
	virtual void BeginPlay() override;
};
