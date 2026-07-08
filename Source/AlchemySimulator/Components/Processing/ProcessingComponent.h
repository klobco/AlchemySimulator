// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataStructHelpers.h"
#include "ItemMetadata.h"
#include "ProcessingComponent.generated.h"

class UDataAssetProcessingMethod;

UCLASS( ClassGroup=(Alchemy), meta=(BlueprintSpawnableComponent) )
class ALCHEMYSIMULATOR_API UProcessingComponent : public UActorComponent
{
	GENERATED_BODY()

UFUNCTION(BlueprintCallable)
FItemInstanceData BuildProcessedInstance(
    const FItemInstanceData& OriginalInstance,
    UDataAssetProcessingMethod* ProcessingMethod,
    float MiniGameQuality
) const;
		
};
