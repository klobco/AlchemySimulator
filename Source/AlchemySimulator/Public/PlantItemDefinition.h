// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinitionBase.h"
#include "PlantItemDefinition.generated.h"

UENUM(BlueprintType)
enum class EIngredientPart : uint8
{
    None,
    Whole,
    Leaf,
    Stem,
    Root,
    Fruit,
    Flower
};

/**
 *
 */
UCLASS()
class ALCHEMYSIMULATOR_API UPlantItemDefinition : public UItemDefinitionBase
{
	GENERATED_BODY()

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Category")
    EIngredientPart Part = EIngredientPart::None;
};
