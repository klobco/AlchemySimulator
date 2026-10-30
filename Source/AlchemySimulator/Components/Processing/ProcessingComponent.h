// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataStructHelpers.h"
#include "ItemMetadata.h"
#include "ProcessingComponent.generated.h"

class UDataAssetProcessingMethod;

UCLASS( ClassGroup=(Alchemy), meta=(BlueprintSpawnableComponent) )
class ALCHEMYSIMULATOR_API UProcessingComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	/**
	 * Apply a processing method to an item instance.
	 *
	 * A pure function of its arguments — it touches no component state — so it is static and
	 * callable as UProcessingComponent::BuildProcessedInstance(...) without an instance.
	 * (It was previously private by accident: GENERATED_BODY() preserves the enclosing access
	 * level, which is private for a class, so nothing outside could call it.)
	 */
	UFUNCTION(BlueprintCallable, Category = "Alchemy|Processing")
	static FItemInstanceData BuildProcessedInstance(
	    const FItemInstanceData& OriginalInstance,
	    UDataAssetProcessingMethod* ProcessingMethod,
	    float MiniGameQuality
	);

	/**
	 * Undo one method on an instance: drops its ProcessingTag and its AppliedMethods entry, and
	 * divides its GeneralPotencyMultiplier back out of ProcessingQuality.
	 *
	 * This exists so a state can be *replaced* rather than stacked -- a drying rack swaps Dried
	 * for Scorched instead of leaving both on the item. Tag, method list and multiplier are
	 * undone together here for the same reason BuildProcessedInstance applies them together:
	 * removing the tag alone would leave the multiplier silently baked in forever.
	 *
	 * Two honest limits. It cannot recover the MiniGameQuality that was folded in alongside the
	 * multiplier, so it is exact only for a method applied at quality 1.0 (which is every
	 * unattended one). And a GeneralPotencyMultiplier of 0 destroyed the value, so ProcessingQuality
	 * is left untouched in that case. Does nothing if the method is not on the instance.
	 */
	UFUNCTION(BlueprintCallable, Category = "Alchemy|Processing")
	static FItemInstanceData RemoveProcessingMethod(
	    const FItemInstanceData& OriginalInstance,
	    UDataAssetProcessingMethod* ProcessingMethod
	);

};
