// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DataAssetAlchemyEfectDefinition.generated.h"

/**
 * Data asset defining an alchemy effect.
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDataAssetAlchemyEfectDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta=(MultiLine=true))
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag EffectTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bIsPositive = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bIsRootCauseTreatment = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bIsSymptomRelief = false;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    bool bIsSideEffect = false;
};
