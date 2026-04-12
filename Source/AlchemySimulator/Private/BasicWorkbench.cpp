// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseTool.h"
#include "ItemDefinitionBase.h"
#include "InventoryComponent.h"
#include "DrawDebugHelpers.h"
#include "BasicWorkbench.h"

ABasicWorkbench::ABasicWorkbench() {

	KnifeStand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Knife stand"));
	KnifeStand->SetupAttachment(Body);

	HerbStand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Herb stand"));
	HerbStand->SetupAttachment(Body);

	toolsInventory->MaxSlots = ToolsLimit;
	herbsInventory->MaxSlots = HerbsLimit;

	herbsInventory->OnInventoryChanged.AddDynamic(this, &ABasicWorkbench::HandleHerbsInvenotyChange);
}

void ABasicWorkbench::AddTool(ABaseTool* tool) {
	Super::AddTool(tool);


	if (!tool) return;

	int32 SlotIndex = toolsInventory->FindFirstEmptySlotIndex();
	if (SlotIndex == INDEX_NONE)
	{
		UE_LOG(LogTemp, Error, TEXT("BasicWorkbench: toolsInventory full"));
		return;
	}

	if (tool->Item->Category == EItemCategory::Knife)
	{
		FName SocketName = FName(*FString::Printf(TEXT("Knife%d"), SlotIndex));
		UE_LOG(LogTemp, Error, TEXT("Socket name is %s"), *SocketName.ToString());

		tool->AttachToComponent(
			KnifeStand,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			SocketName
		);

		tool->SetActorRelativeRotation(FRotator(90.f, 0.f, 0.f));
	}
	else
	{
		FName SocketName = FName(*FString::Printf(TEXT("Socket%d"), SlotIndex));
		UE_LOG(LogTemp, Error, TEXT("Socket name is %s"), *SocketName.ToString());

		tool->AttachToComponent(
			Body,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			SocketName
		);
	}

	Tools.Add(SlotIndex, tool);
	toolsInventory->AddItem(tool->Item, 1, tool->Instance);
}

void ABasicWorkbench::RemoveTool(ABaseTool* tool) {
	if (!tool) return;

	// Find the slot index (key) for this tool in the Tools map
	int32 SlotIndex = INDEX_NONE;
	for (auto& Pair : Tools)
	{
		if (Pair.Value == tool)
		{
			SlotIndex = Pair.Key;
			break;
		}
	}

	if (SlotIndex == INDEX_NONE) return;

	// Remove by index — RemoveSlotItem searches by item pointer (first match)
	// which clears the wrong slot when two tools share the same item type
	toolsInventory->RemoveSlotAtIndex(SlotIndex);

	// Remove from the Tools map
	Tools.Remove(SlotIndex);

	// Detach from socket and destroy the actor
	tool->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	tool->Destroy();
}


void ABasicWorkbench::HandleHerbsInvenotyChange() {

	
	if (herbsInventory->CountNonEmptySlots() == Herbs.Num()) //Herb Moved
	{
		// Find out where it was moved
		// Find out from where it was moved
		// Change the position of the herb by moving it to different socket
		// Change the map index of that herb to corespond with the correct iventory index
	}
	else if (herbsInventory->CountNonEmptySlots() > Herbs.Num()) //Herb added
	{
		// Find out where was herb added
		// Create a new Herb instance and set the position to empty socket
		for (int i = 0; i < herbsInventory->MaxSlots; i++)
		{
			if (!herbsInventory->Slots[i].IsEmpty() && !Herbs.Contains(i))
			{
				// Create Herb
			}
		}
	}
	else if (herbsInventory->CountNonEmptySlots() < Herbs.Num()) // Herb removed
	{
		// Find out from where was Herb removed
	}
}