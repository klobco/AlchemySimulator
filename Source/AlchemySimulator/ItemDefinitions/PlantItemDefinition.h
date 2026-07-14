// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "DataStructHelpers.h"
#include "PlantItemDefinition.generated.h"

/**
 *
 */
UCLASS()
class ALCHEMYSIMULATOR_API UPlantItemDefinition : public UItemDefinitionBase
{
	GENERATED_BODY()

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTagContainer Biomes;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FPlantPartHarvestData> HarvestableParts;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float Rarity = 1.0f;

    virtual bool CanUseItem_Implementation(const FItemUseContext& Context) const override;
	virtual FItemUseResult UseItem_Implementation(const FItemUseContext& Context) const override;
};
