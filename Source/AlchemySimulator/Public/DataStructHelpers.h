// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "DataStructHelpers.generated.h"

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