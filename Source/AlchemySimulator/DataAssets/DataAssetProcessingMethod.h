// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DataAssetProcessingMethod.generated.h"


class UDataAssetAlchemyEfectDefinition;
class UDataAssetSubstanceDefinition;

USTRUCT(BlueprintType)
struct FProcessingEffectModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UDataAssetAlchemyEfectDefinition> Effect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Multiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float AdditiveBonus = 0.0f;
};

USTRUCT(BlueprintType)
struct FSubstanceTransformationRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UDataAssetSubstanceDefinition> FromSubstance = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UDataAssetSubstanceDefinition> ToSubstance = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Chance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float AmountMultiplier = 1.0f;
};


/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDataAssetProcessingMethod : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag ProcessingTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float GeneralPotencyMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ToxicityMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float StabilityMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FProcessingEffectModifier> EffectModifiers;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FSubstanceTransformationRule> TransformationRules;
};
