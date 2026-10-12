// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IInteractable.h"
#include "IToolActionTarget.h"
#include "ItemMetadata.h"
#include "GameplayTagContainer.h"
#include "DataAssets/DataAssetPlantPart.h"
#include "PlantPart.generated.h"

UCLASS()
class ALCHEMYSIMULATOR_API APlantPart : public AActor, public IInteractable, public IToolActionTarget
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlantPart();

	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemInstanceData Instance;

	/**
	 * How many inventory units this one actor stands for. 1 for a freshly harvested part or one
	 * placed on a workbench; a stack dropped out of an inventory spawns a single actor carrying
	 * the whole count. Processing assumes 1, which holds because the workbench drag path only
	 * ever moves one unit onto the table.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<const UDataAssetPlantPart> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* OverlayMaterialInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	EHerbStatus HerbStatus;

	UPROPERTY()
	class ABasicWorkbench* ParentWorkbench = nullptr;

	/**
	 * Runtime entry point: point this actor at a part definition and take its mesh, material and
	 * scale from it. One BP_PlantPart serves every harvested part in the game.
	 */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void SetPartDefinition(const UDataAssetPlantPart* InDef, const FItemInstanceData& InInstance);

	UFUNCTION()
	void HandleBeginCursorOver(UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleClicked(UPrimitiveComponent* Component, FKey ButtonPressed);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Pushes Item's mesh/material/scale onto Body. Safe to call in the editor and at runtime. */
	void ApplyPartVisuals();

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Makes editing Item in the details panel show the right mesh in the viewport immediately. */
	virtual void OnConstruction(const FTransform& Transform) override;

	virtual FText   GetInteractPrompt_Implementation() const override;
	virtual FVector GetInteractWorldLocation_Implementation() const override;
	virtual bool    CanInteract_Implementation(APawn* By) const override;
	virtual void    Interact_Implementation(APawn* By) override;
	virtual void    OnFocStart_Implementation(APawn* By) override;
	virtual void    OnFocEnd_Implementation(APawn* By) override;

	virtual bool CanAcceptToolAction_Implementation(UToolItemDefinition* Tool, const FToolAction& Action, UPrimitiveComponent* HitComponent) const override;
	virtual void ApplyToolActionResult_Implementation(UToolItemDefinition* Tool, const FToolAction& Action, const FMinigameResult& Result, UPrimitiveComponent* HitComponent) override;

};
