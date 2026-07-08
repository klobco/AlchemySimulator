// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "ToolItemDefinition.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EToolCategory : uint8
{
    None,
    Knife,
    Pestle
};

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

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Category")
    EToolCategory ToolCategory = EToolCategory::None;
};
