// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseTool.h"
#include "ItemDefinitionBase.h"
#include "InventoryComponent.h"
#include "BasePlant.h"
#include "DrawDebugHelpers.h"
#include "BasicWorkbench.h"

ABasicWorkbench::ABasicWorkbench() {

	KnifeStand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Knife stand"));
	KnifeStand->SetupAttachment(Body);

	HerbStand = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Herb stand"));
	HerbStand->SetupAttachment(Body);

	toolsInventory->MaxSlots = ToolsLimit;
	herbsInventory->MaxSlots = HerbsLimit;

	
}

void ABasicWorkbench::BeginPlay()
{
	Super::BeginPlay();

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

void ABasicWorkbench::CreateHerb(FInventorySlot InSlot, int32 indexToAdd) {

	UE_LOG(LogTemp, Error, TEXT("Creating Herb"));

	UWorld* World = this->GetWorld();
	if (!World) return;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FRotator SpawnRotation(0.f, 0.f, 0.f);

	ABasePlant* SpawnedPlant = World->SpawnActor<ABasePlant>(
		InSlot.Item->WorldItem,
		this->GetActorLocation(),
		SpawnRotation,
		Params
	);

	if (SpawnedPlant)
	{
		TArray<UStaticMeshComponent*> MeshComponents;
		SpawnedPlant->GetComponents<UStaticMeshComponent>(MeshComponents);

		SpawnedPlant->GetRootComponent()->SetWorldScale3D(FVector(0.07f));
		SpawnedPlant->Tags.Empty();
		for (UStaticMeshComponent* MeshComp : MeshComponents)
		{
			if (!MeshComp) continue;

			MeshComp->SetSimulatePhysics(false);
			MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}


		FName SocketName = FName(*FString::Printf(TEXT("Herb%d"), indexToAdd));
		UE_LOG(LogTemp, Error, TEXT("Herb Socket name is %s"), *SocketName.ToString());

		SpawnedPlant->AttachToComponent(
			HerbStand,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			SocketName
		);


		Herbs.Add(indexToAdd, SpawnedPlant);
	}
}

void ABasicWorkbench::RemoveHerb(int32 indexToRemove) {


	if (ABasePlant** herbToRemove = Herbs.Find(indexToRemove)){

		ABasePlant* Herb = *herbToRemove;
		if (Herb)
		{
			Herb->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			Herb->Destroy();
		}
		Herbs.Remove(indexToRemove);
	}
}
void ABasicWorkbench::MoveHerb(int32 from, int32 to) {


	if (ABasePlant** herbToMove = Herbs.Find(from)) {

		ABasePlant* Herb = *herbToMove;
		if (Herb)
		{
			Herb->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
			Herbs.Add(to, Herb);

			FName SocketName = FName(*FString::Printf(TEXT("Herb%d"), to));
			UE_LOG(LogTemp, Error, TEXT("Herb Socket name is %s"), *SocketName.ToString());

			Herb->AttachToComponent(
				HerbStand,
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				SocketName
			);
		}

		Herbs.Remove(from);
		
	}
}


void ABasicWorkbench::HandleHerbsInvenotyChange() {

	UE_LOG(LogTemp, Error, TEXT("Herb Inventory change event"));

	
	if (herbsInventory->CountNonEmptySlots() == Herbs.Num()) //Herb Moved
	{

		int32 removedIndex = INDEX_NONE;
		int32 addedIndex = INDEX_NONE;

		for (int i = 0; i < herbsInventory->MaxSlots; i++)
		{
			if (!herbsInventory->Slots[i].IsEmpty() && !Herbs.Contains(i))
			{
				addedIndex = i;
			}
			else if (herbsInventory->Slots[i].IsEmpty() && Herbs.Contains(i)) {
				removedIndex = i;
			}
		}

		if (removedIndex != INDEX_NONE && addedIndex != INDEX_NONE)
		{
			MoveHerb(removedIndex, addedIndex);
		}

	}
	else if (herbsInventory->CountNonEmptySlots() > Herbs.Num()) //Herb added
	{
		for (int i = 0; i < herbsInventory->MaxSlots; i++)
		{
			if (!herbsInventory->Slots[i].IsEmpty() && !Herbs.Contains(i))
			{
				CreateHerb(herbsInventory->Slots[i], i);
			}
		}
	}
	else if (herbsInventory->CountNonEmptySlots() < Herbs.Num()) // Herb removed
	{
		for (int i = 0; i < herbsInventory->MaxSlots; i++)
		{
			if (herbsInventory->Slots[i].IsEmpty() && Herbs.Contains(i))
			{
				RemoveHerb(i);
			}
		}
	}
}