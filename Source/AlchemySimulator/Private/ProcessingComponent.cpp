// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssetPlantPart.h"
#include "PlantItemDefinition.h"
#include "DataAssetPlantPart.h"
#include "ProcessingComponent.h"

FProcessedIngredient UProcessingComponent::ProcessIngredient(
    const FIngredientInstance& Ingredient,
    UDataAssetProcessingMethod* ProcessingMethod,
    float MiniGameQuality
)
{
    FProcessedIngredient Result;
    Result.BaseIngredient = Ingredient;
    Result.ProcessingQuality = FMath::Clamp(MiniGameQuality, 0.0f, 1.5f);

    if (ProcessingMethod)
    {
        Result.AppliedProcesses.Add(ProcessingMethod);
    }

    if (!Ingredient.PlantPart)
    {
        return Result;
    }

    for (const FSubstanceAmount& SubstanceAmount : Ingredient.PlantPart->HarvestableParts->PlantPartDefinition->Substances)
    {
        if (!SubstanceAmount.Substance)
        {
            continue;
        }

        FSubstanceAmount FinalAmount;
        FinalAmount.Substance = SubstanceAmount.Substance;

        float Amount = SubstanceAmount.Amount;
        Amount *= Ingredient.Quality;
        Amount *= Ingredient.Freshness;
        Amount *= Result.ProcessingQuality;

        if (ProcessingMethod)
        {
            Amount *= ProcessingMethod->GeneralPotencyMultiplier;
        }

        FinalAmount.Amount = Amount;
        Result.FinalSubstances.Add(FinalAmount);
    }

    return Result;
}