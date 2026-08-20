// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "DataStructHelpers.h"
#include "DataAssetPlantPart.generated.h"

class UMaterialInterface;

/**
 *
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDataAssetPlantPart : public UItemDefinitionBase
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag PlantPartTag;

	/** Mesh this part shows as a harvested world actor, and its default mesh on the growing plant. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|World")
	TObjectPtr<UStaticMesh> WorldMesh = nullptr;

	/** Optional material swap, e.g. for a tinted or emissive variant of a shared mesh. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|World")
	TObjectPtr<UMaterialInterface> WorldMaterialOverride = nullptr;

	/**
	 * Tool actions that harvest this part off the growing plant, and that work on it once
	 * harvested (e.g. ToolAction.Cut for a leaf, ToolAction.Crush for a root). Per-part, so one
	 * plant can need two different tools. This lives on the part, not the actor, because a single
	 * shared BP_Plant would otherwise force every species to accept the same actions.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool")
	FGameplayTagContainer AcceptedToolActions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FSubstanceAmount> Substances;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseQuality = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Alchemy")
	bool bCanBeProcessed = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Alchemy")
	FGameplayTagContainer AllowedProcessingTags;

	/**
	 * The single gate for "may this tool action be performed on this part?".
	 *
	 * Two conditions, both authored on this asset: the verb must be in AcceptedToolActions, and
	 * if the action carries a processing method, that method's tag must be in AllowedProcessingTags.
	 * Both ABasePlant (harvesting off the growing plant) and APlantPart (working a part on the
	 * table) call this, so the two moments can never drift apart.
	 *
	 * ToolAction.* is a verb ("crush this") and Processing.* is the state that results
	 * ("crushed"), which is why they are two containers and not one.
	 */
	UFUNCTION(BlueprintCallable, Category = "Tool")
	bool AcceptsToolAction(const FToolAction& Action) const;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
