// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Stations/BasicInteractableStationObject.h"
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

	UPROPERTY(EditAnywhere, Category = "tools")
	UInventoryComponent* toolsInventory;

	UPROPERTY(EditAnywhere, Category = "tools")
	TMap<int32, class ABaseTool* > Tools;

	UPROPERTY(VisibleAnywhere, Category = "tools")
	int32 ActiveToolIndex = -1;

	UPROPERTY(EditAnywhere, Category = "tools")
	int32 ToolsLimit = 4;

	UPROPERTY(EditAnywhere, Category = "tools")
	class UInventoryComponent* herbsInventory;

	UPROPERTY(EditAnywhere, Category = "tools")
	int32 HerbsLimit = 12;

	/**
	 * Clamp DesiredLocation so the whole of Actor's bounds stays inside MovingZone.
	 * Accounts for meshes whose pivot isn't at their bounds centre. Z is left alone —
	 * the drag plane sets height and physics settles the item onto the table.
	 */
	FVector ClampActorToWorkbench(const AActor* Actor, const FVector& DesiredLocation) const;

	UFUNCTION(BlueprintCallable)
	bool TryPlaceDraggedItem(class UInvDragOperation* DragOp, const FHitResult& Hit);

	UFUNCTION(BlueprintCallable)
	bool RemoveHerbItem(AActor* HerbToRemove);

	UFUNCTION(BlueprintCallable)
	bool CanAcceptDraggedItem(const class UItemDefinitionBase* ItemDef, const FItemInstanceData& Instance) const;

	UPROPERTY(EditAnywhere, Category = "herbs")
	TArray<class AActor*> HerbsOnTable;

	UFUNCTION(BlueprintCallable)
	void AddTool(ABaseTool* tool);

	UFUNCTION(BlueprintCallable)
	void RemoveTool(ABaseTool* tool);

	UFUNCTION(BlueprintCallable)
	void SetActiveTool(ABaseTool* tool);

	virtual class UInventoryComponent* GetStationInventory() const override;

protected:
	virtual void BeginPlay() override;
};
