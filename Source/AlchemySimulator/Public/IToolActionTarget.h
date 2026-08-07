// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ItemMetadata.h"
#include "DataStructHelpers.h"
#include "IToolActionTarget.generated.h"

class UToolItemDefinition;
class UPrimitiveComponent;

UINTERFACE(BlueprintType)
class ALCHEMYSIMULATOR_API UToolActionTarget : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by anything a workbench tool can be used on (plants, plant parts, ...).
 *
 * Targets never look at which tool they were hit with. They answer two questions:
 * "do I accept this action?" and "here is the result, apply it". The tool supplies the
 * minigame and processing method via FToolAction, and UMinigameManagerComponent owns the
 * lifetime in between — no target ever binds to a minigame delegate.
 */
class ALCHEMYSIMULATOR_API IToolActionTarget
{
	GENERATED_BODY()

public:
	/**
	 * Can this action be performed on this target right now? Called for each of the tool's
	 * actions in order; the first accepted one runs. HitComponent is the clicked primitive,
	 * so targets made of several meshes can accept per-part (e.g. leaves but not the stem).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tool")
	bool CanAcceptToolAction(UToolItemDefinition* Tool, const FToolAction& Action, UPrimitiveComponent* HitComponent) const;

	/** Apply a finished action. Called once, only for the action that CanAcceptToolAction approved. */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Tool")
	void ApplyToolActionResult(UToolItemDefinition* Tool, const FToolAction& Action, const FMinigameResult& Result, UPrimitiveComponent* HitComponent);
};
