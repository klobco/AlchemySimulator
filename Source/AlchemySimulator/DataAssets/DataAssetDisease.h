// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DataStructHelpers.h"
#include "DataAssetDisease.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDataAssetDisease : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer DiseaseTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer Symptoms;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FTreatmentRequirement> TreatmentRequirements;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTagContainer RelevantNutrients;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float MaxAllowedToxicity = 40.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ProgressionSpeed = 1.0f;
};
