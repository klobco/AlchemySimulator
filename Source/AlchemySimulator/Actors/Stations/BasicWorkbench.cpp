// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Tools/BaseTool.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Actors/Plants/BasePlant.h"
#include "DrawDebugHelpers.h"
#include "InvDragOperation.h"
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h"
#include "Actors/Plants/PlantPart.h"
#include "DataAssets/DataAssetPlantPart.h"
#include "Actors/Stations/BasicWorkbench.h"

ABasicWorkbench::ABasicWorkbench() {

	KnifeStand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Knife stand"));
	KnifeStand->SetupAttachment(Body);

	toolsInventory->MaxSlots = ToolsLimit;
	herbsInventory->MaxSlots = HerbsLimit;


	MovingZone = CreateDefaultSubobject<UBoxComponent>(TEXT("MovingZone"));
	MovingZone->SetupAttachment(RootComponent);
	MovingZone->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

	if (tool->Item->ToolCategory == EToolCategory::Knife)
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
	else if (tool->Item->ToolCategory == EToolCategory::Pestle) {
		FName SocketName = FName(*FString::Printf(TEXT("Tool%d"), SlotIndex));
		UE_LOG(LogTemp, Error, TEXT("Socket name is %s"), *SocketName.ToString());

		tool->AttachToComponent(
			Body,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			SocketName
		);
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
	if (HerbsOnTable.Num() >= 2) return false;

	const UItemDefinitionBase* ItemDef = DragOp->SlotSnapshot.Item;
	if (!ItemDef) return false;
	if (ItemDef->Category != EItemCategory::Herb &&
		ItemDef->Category != EItemCategory::Ingredient)
	{
		return false;
	}

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

	// DrawDebugSphere(GetWorld(), SpawnLocation, 10.f, 12, FColor::Red, true);
	SpawnLocation = Body->GetSocketLocation("ManipulationSocket");

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
				MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}

			HerbsOnTable.Add(SpawnedPlant);

			SpawnedPlant->Item = Cast<UPlantItemDefinition>(CurrentSlot.Item.Get());
			SpawnedPlant->Instance = CurrentSlot.Instance;

			SpawnedPlant->ParentWorkbench = this;
			SpawnedPlant->HerbStatus = EHerbStatus::OnTable;

			SpawnedPlant->SetActorLocation(Body->GetSocketLocation("ManipulationSocket"));
			SpawnedPlant->SetActorRotation(FRotator(0.f, -5.f, 90.f));

			SpawnedPlant->Stem->SetSimulatePhysics(true);

			UE_LOG(LogTemp, Warning, TEXT("Spawned plant %s at location %s and rotation %s"), *SpawnedPlant->GetName(), *SpawnedPlant->GetActorLocation().ToString(), *SpawnedPlant->GetActorRotation().ToString());
		}

		if (APlantPart* SpawnedPlantPart = Cast<APlantPart>(SpawndedActor))
		{
			TArray<UStaticMeshComponent*> MeshComponents;
			SpawnedPlantPart->GetComponents<UStaticMeshComponent>(MeshComponents);

			SpawnedPlantPart->SetActorScale3D(FVector(0.07f));
			SpawnedPlantPart->Tags.Empty();
			for (UStaticMeshComponent* MeshComp : MeshComponents)
			{
				if (!MeshComp) continue;

				MeshComp->SetSimulatePhysics(false);
				MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}

			HerbsOnTable.Add(SpawnedPlantPart);

			SpawnedPlantPart->Item = Cast<UDataAssetPlantPart>(CurrentSlot.Item.Get());
			SpawnedPlantPart->Instance = CurrentSlot.Instance;

			SpawnedPlantPart->ParentWorkbench = this;
			SpawnedPlantPart->HerbStatus = EHerbStatus::OnTable;

			SpawnedPlantPart->SetActorLocation(Body->GetSocketLocation("ManipulationSocket"));
			SpawnedPlantPart->SetActorRotation(FRotator(0.f, -5.f, 90.f));

			SpawnedPlantPart->Body->SetSimulatePhysics(true);

			UE_LOG(LogTemp, Warning, TEXT("Spawned plant part %s at location %s and rotation %s"), *SpawnedPlantPart->GetName(), *SpawnedPlantPart->GetActorLocation().ToString(), *SpawnedPlantPart->GetActorRotation().ToString());
		}


		FInventorySlot RemovedSlot;
		const bool bRemoved = DragOp->SourceInventory->RemoveAt(DragOp->FromIndex, 1, RemovedSlot);
		if (!bRemoved)
		{
			SpawndedActor->Destroy();
			return false;
		}

	return true;
}

bool ABasicWorkbench::RemoveHerbItem(AActor* HerbToRemove)
{
	if (HerbToRemove)
	{
		HerbsOnTable.Remove(HerbToRemove);
		return true;
	}
	return false;
}

FVector ABasicWorkbench::ClampActorToWorkbench(const AActor* Actor, const FVector& DesiredLocation) const
{
	if (!MovingZone || !Actor)
	{
		return DesiredLocation;
	}

	FVector Origin, BoxExtent;
	Actor->GetActorBounds(true, Origin, BoxExtent);

	// The bounds centre is usually offset from the pivot (plant parts pivot at the
	// base), so clamp the centre and convert back to a pivot location afterwards.
	const FVector PivotToCentre = Origin - Actor->GetActorLocation();

	const FTransform AreaTransform = MovingZone->GetComponentTransform();
	FVector LocalCentre = AreaTransform.InverseTransformPosition(DesiredLocation + PivotToCentre);

	// Shrink the allowed region by the item's own half-size so nothing overhangs.
	FVector Allowed = MovingZone->GetScaledBoxExtent() - BoxExtent;
	Allowed.X = FMath::Max(Allowed.X, 0.f);
	Allowed.Y = FMath::Max(Allowed.Y, 0.f);

	LocalCentre.X = FMath::Clamp(LocalCentre.X, -Allowed.X, Allowed.X);
	LocalCentre.Y = FMath::Clamp(LocalCentre.Y, -Allowed.Y, Allowed.Y);
	// Z intentionally not clamped — the drag plane locks height, physics settles it.

	return AreaTransform.TransformPosition(LocalCentre) - PivotToCentre;
}