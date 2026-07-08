// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DataStructHelpers.h"
#include "DataAssetSubstanceDefinition.generated.h"

/**
 * Data asset defining a substance.
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDataAssetSubstanceDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(MultiLine=true))
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag SubstanceTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FAlchemyEffectValue> BaseEffects;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float BaseToxicity = 0.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float HeatStability = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float DryingStability = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float CrushingStability = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTagContainer Properties;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FLinearColor ColorHint = FLinearColor::White;
};
