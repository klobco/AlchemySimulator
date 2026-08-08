// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IInteractable.h"
#include "IToolActionTarget.h"
#include "ItemMetadata.h"
#include "GameplayTagContainer.h"
#include "ItemDefinitions/PlantItemDefinition.h"
#include "DataAssets/DataAssetPlantPart.h"
#include "BasePlant.generated.h"

UCLASS()
class ALCHEMYSIMULATOR_API ABasePlant : public AActor, public IInteractable, public IToolActionTarget
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasePlant();

	/**
	 * The structural body (stem/stipe/trunk). Root component, and the only simulating physics
	 * body — every harvestable part rides it. Shows the mesh of the bIsStructural harvest row.
	 */
	UPROPERTY(EditAnywhere, Category = "components")
	class UStaticMeshComponent* Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float CurrentQuality = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	float CurrentFreshness = 1.0f;

	/** The species. Its HarvestableParts array is the single source of truth for this plant's anatomy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<const UPlantItemDefinition> Item;

	UPROPERTY()
	class ABasicWorkbench* ParentWorkbench;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemInstanceData Instance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* OverlayMaterialInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	EHerbStatus HerbStatus;

	UFUNCTION()
	void HandleBeginCursorOver(UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleClicked(UPrimitiveComponent* Component, FKey ButtonPressed);

	void SetPlantHighlight(bool bEnabled);

	/**
	 * Runtime entry point: point this actor at a species and build its parts. Assigns Item and
	 * Instance, applies WorldScale, then rebuilds the generated part components.
	 */
	UFUNCTION(BlueprintCallable, Category = "Item")
	void InitializeFromDefinition(const UPlantItemDefinition* InDef, const FItemInstanceData& InInstance);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/**
	 * Tears down every generated part component and rebuilds them from Item->HarvestableParts.
	 * Also where the cursor/click delegates are bound — BeginPlay is too early, because on the
	 * workbench path the parts do not exist until the workbench assigns Item.
	 */
	void RebuildPartComponents();

	/** Generated part meshes, owned by this actor and destroyed on every rebuild. */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UStaticMeshComponent>> GeneratedPartComponents;

	/**
	 * Component -> index into Item->HarvestableParts. Stores the row index rather than the part
	 * definition so a single lookup also yields MinYield/MaxYield/HarvestChance. The structural
	 * part is deliberately absent, which is what makes it un-cuttable.
	 */
	UPROPERTY(Transient)
	TMap<TObjectPtr<UStaticMeshComponent>, int32> PartComponentToHarvestRow;

	/** Index of the bIsStructural row in Item->HarvestableParts, or INDEX_NONE. */
	UPROPERTY(Transient)
	int32 StructuralRow = INDEX_NONE;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Makes editing Item in the details panel assemble the plant in the viewport immediately. */
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
