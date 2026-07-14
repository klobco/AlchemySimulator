// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefinitions/ItemDefinitionBase.h"

bool UItemDefinitionBase::CanUseItem_Implementation(const FItemUseContext& Context) const
{
    return Context.User != nullptr
        && Context.Inventory != nullptr
        && !Context.Slot.IsEmpty();
}

FItemUseResult UItemDefinitionBase::UseItem_Implementation(const FItemUseContext& Context) const
{
    FItemUseResult Result;
    Result.bSuccess = false;
    Result.bConsumeOneItem = false;
    Result.Message = FText::FromString(TEXT("This item cannot be used."));
    return Result;
}