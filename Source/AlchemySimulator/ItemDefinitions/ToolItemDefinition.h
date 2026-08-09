// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "ItemMetadata.h"
#include "ToolItemDefinition.generated.h"

class UTexture2D;

/**
 * Physical shape of the tool. Drives where the workbench parents it (knife stand vs. body
 * socket) — see ABasicWorkbench::AddTool. It is deliberately NOT used to decide what a tool
 * does to a target; that is data, see UToolItemDefinition::Actions.
 */
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

    /**
     * Everything this tool can do, in priority order — the first action the clicked target
     * accepts is the one that runs. See UMinigameManagerComponent::TryStartToolAction.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Actions")
    TArray<FToolAction> Actions;

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
