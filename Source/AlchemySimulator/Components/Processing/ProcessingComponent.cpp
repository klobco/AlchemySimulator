// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/DataAssetPlantPart.h"
#include "ItemDefinitions/PlantItemDefinition.h"
#include "DataAssets/DataAssetPlantPart.h"
#include "DataAssets/DataAssetProcessingMethod.h"
#include "Components/Processing/ProcessingComponent.h"

FItemInstanceData UProcessingComponent::BuildProcessedInstance(
    const FItemInstanceData& OriginalInstance,
    UDataAssetProcessingMethod* ProcessingMethod,
    float MiniGameQuality
)
{
    FItemInstanceData Result = OriginalInstance;

    if (!ProcessingMethod)
    {
        return Result;
    }

    const float ClampedQuality = FMath::Clamp(MiniGameQuality, 0.0f, 1.5f);

    Result.bIsProcessed = true;
    Result.ProcessingTags.AddTag(ProcessingMethod->ProcessingTag);
    Result.AppliedMethods.Add(ProcessingMethod);
    Result.ProcessingQuality *= ClampedQuality;
    Result.ProcessingQuality *= ProcessingMethod->GeneralPotencyMultiplier;

    return Result;
}

FItemInstanceData UProcessingComponent::RemoveProcessingMethod(
    const FItemInstanceData& OriginalInstance,
    UDataAssetProcessingMethod* ProcessingMethod
)
{
    FItemInstanceData Result = OriginalInstance;

    if (!ProcessingMethod)
    {
        return Result;
    }

    const int32 Removed = Result.AppliedMethods.RemoveAll(
        [ProcessingMethod](const TObjectPtr<const UDataAssetProcessingMethod>& Applied)
        {
            return Applied == ProcessingMethod;
        });

    if (Removed <= 0)
    {
        return Result;
    }

    Result.ProcessingTags.RemoveTag(ProcessingMethod->ProcessingTag);

    // Once per removed entry, mirroring how each application multiplied it in.
    const float Multiplier = ProcessingMethod->GeneralPotencyMultiplier;
    if (!FMath::IsNearlyZero(Multiplier))
    {
        for (int32 i = 0; i < Removed; ++i)
        {
            Result.ProcessingQuality /= Multiplier;
        }
    }

    // bIsProcessed is "has anything been done to this", so it follows the list rather than
    // staying true forever once set.
    Result.bIsProcessed = Result.AppliedMethods.Num() > 0;

    return Result;
}