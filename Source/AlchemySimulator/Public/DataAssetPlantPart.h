// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinitionBase.h"
#include "DataStructHelpers.h"
#include "DataAssetPlantPart.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDataAssetPlantPart : public UItemDefinitionBase
{
	GENERATED_BODY()
	
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag PlantPartTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FSubstanceAmount> Substances;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseQuality = 1.0f;
};
