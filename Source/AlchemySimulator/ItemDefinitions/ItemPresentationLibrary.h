// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ItemMetadata.h"
#include "ItemPresentationLibrary.generated.h"

class UItemDefinitionBase;


/**
 * The one place that answers "what does this item look like, and what is it called, in the
 * processing state this instance is in". Three surfaces need the same answer (inventory slot,
 * drag visual, table row) and the world actor needs a fourth, so the walk over
 * FItemInstanceData::AppliedMethods lives here rather than inlined at each call site.
 */
UCLASS()
class ALCHEMYSIMULATOR_API UItemPresentationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	

public:

	/** Icon for an item in a given processing state. Never returns null when the item has an Icon. */
	UFUNCTION(BlueprintPure, Category = "Item|Presentation")
	static UTexture2D* ResolveItemIcon(const FItemInstanceData& Instance, const UItemDefinitionBase* ItemDef);

	/** e.g. "Crushed Dried Mint Leaf". */
	UFUNCTION(BlueprintPure, Category = "Item|Presentation")
	static FText ResolveItemDisplayName(const FItemInstanceData& Instance, const UItemDefinitionBase* ItemDef);

	/** Mesh, material and scale for the item's world representation. */
	UFUNCTION(BlueprintPure, Category = "Item|Presentation")
	static void ResolveItemWorldVisuals(const FItemInstanceData& Instance, const UItemDefinitionBase* ItemDef, UStaticMesh*& OutMesh, UMaterialInterface*& OutMaterial, FVector& OutScale);

};
