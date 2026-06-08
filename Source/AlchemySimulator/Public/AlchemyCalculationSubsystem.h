// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataStructHelpers.h"
#include "ItemMetadata.h"
#include "AlchemyCalculationSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UAlchemyCalculationSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="Alchemy")
    FPotionResult CalculatePotionFromSlots(const TArray<FInventorySlot>& IngredientSlots) const;
	
private:
    void AddEffectValue(
        TArray<FAlchemyEffectValue>& Effects,
        UDataAssetAlchemyEfectDefinition* Effect,
        float Value
    ) const;

    float GetProcessingMultiplier(
        const FItemInstanceData& Instance,
        const UDataAssetSubstanceDefinition* Substance
    ) const;

    float GetEffectValueSafe(float BaseValue, float FinalAmount) const;
};
