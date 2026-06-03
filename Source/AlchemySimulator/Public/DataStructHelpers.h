// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "GameplayTagContainer.h"
#include "DataStructHelpers.generated.h"

class UDataAssetAlchemyEfectDefinition;
class UDataAssetSubstanceDefinition;
class UPlantItemDefinition;
class UDataAssetProcessingMethod;
class UDataAssetPlantPart;

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FMinigameResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly)
    float Score = 0.f;

    UPROPERTY(BlueprintReadOnly)
    float QualityMultiplier = 1.f;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FPestleMortarResult
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly)
    float Groundness = 0.f;

    UPROPERTY(BlueprintReadOnly)
    float QualityMultiplier = 0.f;

    UPROPERTY(BlueprintReadOnly)
    int32 PerfectHits = 0;

    UPROPERTY(BlueprintReadOnly)
    int32 Misses = 0;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FAlchemyEffectValue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UDataAssetAlchemyEfectDefinition> Effect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Value = 0.0f;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FSubstanceAmount
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UDataAssetSubstanceDefinition> Substance = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Amount = 1.0f;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FIngredientInstance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UDataAssetPlantPart> PlantPart = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Quality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Freshness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer InstanceTags;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FPlantPartHarvestData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<const UDataAssetPlantPart> PlantPartDefinition = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MinYield = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 MaxYield = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float HarvestChance = 1.0f;
};

USTRUCT(BlueprintType)
struct FProcessedIngredient
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FIngredientInstance BaseIngredient;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<TObjectPtr<UDataAssetProcessingMethod>> AppliedProcesses;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ProcessingQuality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FSubstanceAmount> FinalSubstances;
};