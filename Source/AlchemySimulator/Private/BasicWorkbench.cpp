// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseTool.h"
#include "ItemDefinitionBase.h"
#include "InventoryComponent.h"
#include "BasePlant.h"
#include "InvDragOperation.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "BasicWorkbench.h"

ABasicWorkbench::ABasicWorkbench() {

	KnifeStand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Knife stand"));
	KnifeStand->SetupAttachment(Body);

	toolsInventory->MaxSlots = ToolsLimit;
	herbsInventory->MaxSlots = HerbsLimit;

	DropZone = CreateDefaultSubobject<UBoxComponent>(TEXT("DropZone"));
	DropZone->SetupAttachment(RootComponent);
	DropZone->ComponentTags.Add(TEXT("WorkbenchDropZone"));
	DropZone->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DropZone->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ABasicWorkbench::BeginPlay()
{
	Super::BeginPlay();

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

bool ABasicWorkbench::CanAcceptDraggedItem(const UItemDefinitionBase* ItemDef) const
{
	if (!ItemDef) return false;

	// Tu si daj vlastnú logiku:
	// napr. povolíš len byliny, tools, potion ingredients, atď.
	return true;
}

bool ABasicWorkbench::TryPlaceDraggedItem(UInvDragOperation* DragOp, const FHitResult& Hit)
{
	if (!DragOp) return false;
	if (!DragOp->SourceInventory) return false;
	if (!DragOp->SlotSnapshot.Item) return false;
	if (DragOp->Quantity <= 0) return false;

	const UItemDefinitionBase* ItemDef = DragOp->SlotSnapshot.Item;
	if (!ItemDef) return false;

	if (!CanAcceptDraggedItem(ItemDef))
	{
		return false;
	}

	if (!ItemDef->WorldItem)
	{
		return false;
	}

	if (!DragOp->SourceInventory->Slots.IsValidIndex(DragOp->FromIndex))
	{
		return false;
	}



	const FInventorySlot& CurrentSlot = DragOp->SourceInventory->Slots[DragOp->FromIndex];

	if (CurrentSlot.Quantity < DragOp->Quantity || CurrentSlot.Item != ItemDef)
	{
		return false;
	}

	FVector SpawnLocation = Hit.ImpactPoint;
	FRotator SpawnRotation = GetActorRotation();

	FActorSpawnParameters Params;
	Params.Owner = this;

	AActor* SpawndedActor = GetWorld()->SpawnActor<AActor>(
		ItemDef->WorldItem,
		SpawnLocation,
		SpawnRotation,
		Params
	);

	if (!SpawndedActor)
	{
		return false;
	}

	if (ABasePlant* SpawnedPlant = Cast<ABasePlant>(SpawndedActor))
		{
			TArray<UStaticMeshComponent*> MeshComponents;
			SpawnedPlant->GetComponents<UStaticMeshComponent>(MeshComponents);

			SpawnedPlant->SetActorScale3D(FVector(0.07f));
			SpawnedPlant->Tags.Empty();
			for (UStaticMeshComponent* MeshComp : MeshComponents)
			{
				if (!MeshComp) continue;

				MeshComp->SetSimulatePhysics(false);
				MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}


			SpawnedPlant->AttachToComponent(
				Body,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				"ManipulationSocket"
			);

			HerbsOnTable.Add(SpawnedPlant);

			SpawnedPlant->Item = CurrentSlot.Item;
			SpawnedPlant->Instance = CurrentSlot.Instance;

			SpawnedPlant->ParentWorkbench = this;

			SpawnedPlant->Stem->SetSimulatePhysics(true);
			SpawnedPlant->Stem->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

			SpawnedPlant->HerbStatus = EHerbStatus::OnTable;
			SpawnedPlant->SetActorRelativeRotation(FRotator(0.f, -5.f, 90.f));
		}


		FInventorySlot RemovedSlot;
		const bool bRemoved = DragOp->SourceInventory->RemoveAt(DragOp->FromIndex, DragOp->Quantity, RemovedSlot);
		if (!bRemoved)
		{
			SpawndedActor->Destroy();
			return false;
		}

	return true;
}
