// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tasks/StateTreeAITask.h"
#include "ItemMetadata.h"
#include "STT_UseItem.generated.h"

class AAIController;


USTRUCT()
struct ALCHEMYSIMULATOR_API FSTT_UseItemTaskInstanceData
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere, Category = "Context")
    TObjectPtr<AAIController> AIController = nullptr;

    UPROPERTY(EditAnywhere, Category = "Parameter")
    FInventorySlot ItemToUse;
};


/**
 * Samotný StateTree Task.
 */
USTRUCT(meta = (
    DisplayName = "Use Item",
    Category = "Item"
))
struct ALCHEMYSIMULATOR_API FSTT_UseItemTask
    : public FStateTreeAITaskBase
{
    GENERATED_BODY()

public:

    using FInstanceDataType = FSTT_UseItemTaskInstanceData;

	FSTT_UseItemTask()
    {
        bShouldCallTick = false;
        bShouldCallTickOnlyOnEvents = false;
    }

    virtual const UStruct* GetInstanceDataType() const override
    {
        return FInstanceDataType::StaticStruct();
    }


    virtual EStateTreeRunStatus EnterState(
        FStateTreeExecutionContext& Context,
        const FStateTreeTransitionResult& Transition
    ) const override;
};