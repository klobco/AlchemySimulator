// Fill out your copyright notice in the Description page of Project Settings.


#include "PotionItemDefinition.h"
#include "Components/Disease/PatientConditionComponent.h"

UPotionItemDefinition::UPotionItemDefinition()
{
	Category = EItemCategory::Potion;
}


bool UPotionItemDefinition::CanUseItem_Implementation(const FItemUseContext& Context) const
{
	return true;
}


FItemUseResult UPotionItemDefinition::UseItem_Implementation(const FItemUseContext& Context) const
{

	UE_LOG(LogTemp, Warning, TEXT("Using potion item: %s"), *GetName());

	if(!Context.Slot.Instance.bIsPotion) return FItemUseResult{false, false, FText::FromString(TEXT("This item is not a potion."))};

	if(!Context.TargetActor) return FItemUseResult{false, false, FText::FromString(TEXT("No target actor specified."))};

	if(!Context.TargetActor->FindComponentByClass<UPatientConditionComponent>())
	{
		return FItemUseResult{false, false, FText::FromString(TEXT("Target actor does not have a patient condition component."))};
	}

	FTreatmentResult TreatmentResult = Context.TargetActor->FindComponentByClass<UPatientConditionComponent>()->ApplyPotion(Context.Slot.Instance);


	FItemUseResult Result;
	Result.bSuccess = true;
	Result.bConsumeOneItem = true;
	Result.Message = FText::FromString(TEXT("This potion cannot be used."));
	return Result;
}
