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

};
