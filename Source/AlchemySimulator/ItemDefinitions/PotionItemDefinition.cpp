// Fill out your copyright notice in the Description page of Project Settings.


#include "PotionItemDefinition.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Components/Disease/PatientConditionComponent.h"

UPotionItemDefinition::UPotionItemDefinition()
{
	Category = EItemCategory::Potion;
	bStackable = false;
    MaxStackSize = 1;
}


bool UPotionItemDefinition::CanUseItem_Implementation(const FItemUseContext& Context) const
{
	return true;
}


FItemUseResult UPotionItemDefinition::UseItem_Implementation(const FItemUseContext& Context) const
{

	if(!Context.Slot.Instance.bIsPotion) return FItemUseResult{false, false, FText::FromString(TEXT("This item is not a potion."))};

	if(!Context.TargetActor) return FItemUseResult{false, false, FText::FromString(TEXT("No target actor specified."))};

	if(!Context.TargetActor->FindComponentByClass<UPatientConditionComponent>())
	{
		return FItemUseResult{false, false, FText::FromString(TEXT("Target actor does not have a patient condition component."))};
	}

	FTreatmentResult TreatmentResult = Context.TargetActor->FindComponentByClass<UPatientConditionComponent>()->ApplyPotion(Context.Slot.Instance);

	UE_LOG(LogTemp, Warning, TEXT("Potion applied. Success: %s"), TreatmentResult.bCured ? TEXT("true") : TEXT("false"));

	FInventorySlot RemovedSlot;
	Context.Inventory->RemoveAt(Context.SlotIndex, 1, RemovedSlot);

	FItemUseResult Result;
	Result.bSuccess = true;
	Result.bConsumeOneItem = true;
	Result.Message = FText::FromString(TEXT("Successfully Used the potion"));
	return Result;
}
