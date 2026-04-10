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

	UFUNCTION(BlueprintCallable)
	virtual void AddTool(ABaseTool* tool) override;

	UFUNCTION(BlueprintCallable)
	virtual void RemoveTool(ABaseTool* tool) override;

	UFUNCTION(BlueprintCallable)
	void AddHerb(FInventorySlot plant);

	UFUNCTION(BlueprintCallable)
	void RemoveHerb(FInventorySlot plant);
	
};
