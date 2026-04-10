// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemMetadata.h"
#include "Blueprint/DragDropOperation.h"
#include "InvDragOperation.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UInvDragOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
    UPROPERTY(BlueprintReadOnly) class UInventoryWidget* SourceWidget = nullptr;
    UPROPERTY(BlueprintReadOnly) class UInventoryComponent* SourceInventory = nullptr;
    UPROPERTY(BlueprintReadOnly) int32 FromIndex = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) int32 Quantity = 0;
    UPROPERTY(BlueprintReadOnly) FInventorySlot SlotSnapshot;
};
