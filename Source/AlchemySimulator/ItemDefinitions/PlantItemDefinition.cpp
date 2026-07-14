// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefinitions/PlantItemDefinition.h"

bool UPlantItemDefinition::CanUseItem_Implementation(const FItemUseContext& Context) const
{
	return true;
}


FItemUseResult UPlantItemDefinition::UseItem_Implementation(const FItemUseContext& Context) const
{
    UE_LOG(LogTemp, Warning, TEXT("Using plant item: %s"), *GetName());
    
	FItemUseResult Result;
	Result.bSuccess = false;
	Result.bConsumeOneItem = false;
	Result.Message = FText::FromString(TEXT("This potion cannot be used."));
	return Result;
}