// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BasicInteractableStationObject.h"
#include "ItemMetadata.h"
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UBoxComponent> DropZone;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<class UBoxComponent> MovingZone;

	FVector ClampLocationToWorkbench(const FVector& WorldLocation) const;

	UFUNCTION(BlueprintCallable)
	bool TryPlaceDraggedItem(class UInvDragOperation* DragOp, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable)
	bool CanAcceptDraggedItem(const class UItemDefinitionBase* ItemDef) const;

	UPROPERTY(EditAnywhere, Category = "herbs")
	TArray<class ABasePlant*> HerbsOnTable;

	UFUNCTION(BlueprintCallable)
	virtual void AddTool(ABaseTool* tool) override;

	UFUNCTION(BlueprintCallable)
	virtual void RemoveTool(ABaseTool* tool) override;
	
protected:
	virtual void BeginPlay() override;
};
