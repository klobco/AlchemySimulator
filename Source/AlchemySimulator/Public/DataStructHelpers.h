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
class UDataAssetDisease;

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
struct ALCHEMYSIMULATOR_API FProcessedIngredient
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

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FPotionResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FAlchemyEffectValue> FinalEffects;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Toxicity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Purity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Stability = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FLinearColor PotionColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FTreatmentRequirement
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UDataAssetAlchemyEfectDefinition> RequiredEffect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float RequiredValue = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bIsMandatory = true;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FPatientBodyState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Immunity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Vitality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ToxicResistance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float RecoveryRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TMap<FGameplayTag, float> Nutrients;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FPatientCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UDataAssetDisease> Disease = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FPatientBodyState BodyState;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Severity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Progress = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer VisibleSymptoms;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FTreatmentResult
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bCured = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    bool bWorsened = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float ProgressChange = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FGameplayTagContainer NewSymptoms;
};