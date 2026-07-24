// Fill out your copyright notice in the Description page of Project Settings.


#include "STT_UseItem.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Components/Disease/PatientConditionComponent.h"
#include "StateTreeExecutionContext.h"

EStateTreeRunStatus FSTT_UseItemTask::EnterState(
    FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition
) const
{
    // Získame Instance Data patriace tomuto konkrétnemu tasku.
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!IsValid(InstanceData.AIController))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Use Item failed: AI Controller is not valid.")
        );

        return EStateTreeRunStatus::Failed;
    }

    ACharacter* ControlledCharacter = Cast<ACharacter>(InstanceData.AIController->GetPawn());
    if (!IsValid(ControlledCharacter))
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("Use Item failed: Controlled character is not valid.")
        );

        return EStateTreeRunStatus::Failed;
    }

    //TODO : Implement the logic for using the item here. This may involve checking the inventory, applying effects, etc.


    return EStateTreeRunStatus::Succeeded;
}