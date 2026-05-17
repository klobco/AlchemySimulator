// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinitionBase.h"
#include "ToolItemDefinition.generated.h"

class UTexture2D;

/**
 *
 */
UCLASS()
class ALCHEMYSIMULATOR_API UToolItemDefinition : public UItemDefinitionBase
{
	GENERATED_BODY()

public:

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Workbench")
    TObjectPtr<UTexture2D> WorkbenchCursor = nullptr;
};
