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
    Result.ProcessingQuality *= ClampedQuality;
    Result.ProcessingQuality *= ProcessingMethod->GeneralPotencyMultiplier;

    return Result;
}