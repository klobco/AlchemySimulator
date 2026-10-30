// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/InventoryComponent.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "ItemDefinitions/PlantItemDefinition.h"
#include "ItemDefinitions/ToolItemDefinition.h"
#include "ItemDefinitions/PotionItemDefinition.h"
#include "DataAssets/DataAssetPlantPart.h"
#include "DataAssets/DataAssetDryingProfile.h"
#include "Actors/Plants/BasePlant.h"
#include "Actors/Plants/PlantPart.h"
#include "Actors/Tools/BaseTool.h"
#include "Actors/Potions/BasePotion.h"
#include "Components/PrimitiveComponent.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	Slots.SetNum(MaxSlots);

	// ...
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Slots.Num() != MaxSlots)
	{
		Slots.SetNum(MaxSlots);
	}

	// Name the owner and the component: a bare count cannot be attributed when several
	// inventories exist in one level (two workbenches x two each, the rack, the player).
	UE_LOG(LogTemp, Log, TEXT("[Inventory] %s.%s: %d slots (MaxSlots %d, StackSizeLimit %d)"),
		*GetNameSafe(GetOwner()), *GetName(), Slots.Num(), MaxSlots, StackSizeLimit);
}


bool UInventoryComponent::AddItem(const UItemDefinitionBase* Item, int32 Quantity, const FItemInstanceData& Instance) {

	// Null check first: the whitelist test dereferences Item->Category.
	if (Item == nullptr || Quantity <= 0)
	{
		return false;
	}

	if (!CategoryWhitelist.IsEmpty() && !CategoryWhitelist.Contains(Item->Category)) return false;

	// One path for stackable and non-stackable alike: a non-stackable item simply has an
	// effective cap of 1, so the fill loops below place one per slot. The old separate
	// branch ignored MaxStackSize and wrote the whole Quantity into a single slot.
	if (FreeStorageForItem(Item, Instance) < Quantity)
	{
		UE_LOG(LogTemp, Error, TEXT("Inventory full"));
		return false;
	}

	const int32 Cap = GetEffectiveMaxStackSize(Item);
	int32 ItemsToAdd = Quantity;

	// Top up existing part-filled stacks first.
	while (ItemsToAdd > 0)
	{
		const int32 StackSlot = FindFirstStackableSlotIndex(Item, Instance);
		if (StackSlot == -1) break;

		const int32 AddNumber = FMath::Min(Cap - Slots[StackSlot].Quantity, ItemsToAdd);
		if (AddNumber <= 0) break;

		Slots[StackSlot].Quantity += AddNumber;
		ItemsToAdd -= AddNumber;
	}

	// Then open fresh slots, Cap at a time.
	while (ItemsToAdd > 0)
	{
		const int32 EmptySlot = FindFirstEmptySlotIndex();
		if (EmptySlot == -1) break;

		FInventorySlot NewSlot;
		NewSlot.Instance = Instance;
		NewSlot.Item = Item;
		NewSlot.Quantity = FMath::Min(Cap, ItemsToAdd);

		Slots[EmptySlot] = NewSlot;
		ItemsToAdd -= NewSlot.Quantity;
	}

	// One add can touch several slots, so no single index is honest - INDEX_NONE means
	// "full refresh". The old GetSlotIndex(Item) reported the first slot holding that
	// definition, which was rarely the slot that changed.
	OnInventoryChangedType.Broadcast(EInventoryChangeType::ItemAdded, INDEX_NONE);
	return true;
}

bool UInventoryComponent::RemoveItem(const UItemDefinitionBase* Item, int32 Quantity, const FItemInstanceData& Instance) {
	return true;
}

bool UInventoryComponent::RemoveSlotAtIndex(int32 Index)
{
	if (!Slots.IsValidIndex(Index) || Slots[Index].IsEmpty()) return false;

	Slots[Index] = FInventorySlot();
	OnInventoryChangedType.Broadcast(EInventoryChangeType::ItemRemoved, Index);
	return true;
}

bool UInventoryComponent::RemoveAt(int32 FromIdx, int32 Quantity, FInventorySlot& OutRemoved)
{
	OutRemoved = FInventorySlot{};
	if (!Slots.IsValidIndex(FromIdx) || Quantity <= 0) return false;

	FInventorySlot& S = Slots[FromIdx];
	const int32 Take = FMath::Min(Quantity, S.Quantity);
	if (Take <= 0) return false;

	
	OutRemoved.Instance = S.Instance;
	OutRemoved.Quantity = Take;
	OutRemoved.Item = S.Item;

	
	S.Quantity -= Take;

	if (S.Quantity <= 0)
	{
		Slots[FromIdx] = FInventorySlot{};
	}

	// Both paths are removals, and both must report: emptying the last herb out of a slot
	// is the event a listener most needs, and it used to return before broadcasting.
	OnInventoryChangedType.Broadcast(EInventoryChangeType::ItemRemoved, FromIdx);
	return true;
}

bool UInventoryComponent::RemoveSlotItem(const FInventorySlot Item) {

	if (Item.IsEmpty()) return false;

	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i].Item == Item.Item && Slots[i].Instance.Quality == Item.Instance.Quality)
		{
			Slots[i] = FInventorySlot();
			OnInventoryChangedType.Broadcast(EInventoryChangeType::ItemRemoved, i);
			return true;
		}
	}
	return false;
}

bool UInventoryComponent::HasItem(const UItemDefinitionBase* Item, int32 Quantity, const FItemInstanceData& Instance) const {
	return true;
}

int32 UInventoryComponent::CountItem(const UItemDefinitionBase* Item, const FItemInstanceData& Instance) const {


	return 0;
}

bool UInventoryComponent::CanStackTogether(const FInventorySlot& A, const FInventorySlot& B) const {

    if (!A.Item || !B.Item) return false;
    if (A.Item != B.Item) return false;
    // Same definition by now, so one query covers both slots. A cap of 1 means this
    // container never merges - the drying rack's one-per-hook rule.
    if (GetEffectiveMaxStackSize(A.Item) <= 1) return false;

    // Drying compares the resolved STAGE, not the raw DryingMinutes: two leaves hung a minute
    // apart would otherwise never merge and the bag fills up with near-identical stacks. The
    // stage is also what the player actually sees. A merge keeps the destination slot's Instance
    // (every merge path only adds Quantity), so the incoming item adopts the destination's
    // progress within the band.
    int32 StageA = INDEX_NONE;
    int32 StageB = INDEX_NONE;
    float ProgressUnused = 0.f;
    UDataAssetDryingProfile::ResolveStageFor(A.Instance, A.Item, StageA, ProgressUnused);
    UDataAssetDryingProfile::ResolveStageFor(B.Instance, B.Item, StageB, ProgressUnused);
    if (StageA != StageB) return false;

    return A.Instance.Quality == B.Instance.Quality
        && FMath::IsNearlyEqual(A.Instance.Freshness, B.Instance.Freshness)
        && A.Instance.ProcessingTags == B.Instance.ProcessingTags
        && FMath::IsNearlyEqual(A.Instance.ProcessingQuality, B.Instance.ProcessingQuality)
        && A.Instance.bIsPotion == B.Instance.bIsPotion;
}

bool UInventoryComponent::UpdateSlotInstance(int32 Index, const FItemInstanceData& Instance)
{
    if (!Slots.IsValidIndex(Index) || Slots[Index].IsEmpty())
    {
        return false;
    }

    Slots[Index].Instance = Instance;

    // One slot changed, so the index goes out with it: a listener that rebuilt everything here
    // would tear down and recreate its whole grid on every game minute a rack is running.
    OnInventoryChangedType.Broadcast(EInventoryChangeType::ItemUpdated, Index);
    return true;
}

bool UInventoryComponent::IsFull() const {
	int32 counter = 0;

	for (auto& Slot : Slots)
	{
		if (!Slot.IsEmpty())
		{
			counter++;
		}
	}

	return counter >= MaxSlots;
}

int32 UInventoryComponent::FindFirstEmptySlotIndex() const {

	for (int32 Index = 0; Index != Slots.Num(); ++Index)
	{
		if (Slots.IsValidIndex(Index) && Slots[Index].IsEmpty())
		{
			return Index;
		}
	}
	return -1;
}

int32 UInventoryComponent::FindFirstStackableSlotIndex(const UItemDefinitionBase* Item, const FItemInstanceData& Instance) const {

	if (Item == nullptr)
	{
		return -1;
	}

	FInventorySlot TempSlot;
	TempSlot.Item = Item;
	TempSlot.Instance = Instance;

	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (CanStackTogether(Slots[Index], TempSlot))
		{
			if (Slots[Index].Quantity < GetEffectiveMaxStackSize(Slots[Index].Item)) {
				return Index;
			}
		}
	}

	return -1;
}

int32 UInventoryComponent::FreeStorageForItem(const UItemDefinitionBase* Item, const FItemInstanceData& Instance) const {

	if (Item == nullptr)
	{
		return 0;
	}

	FInventorySlot TempSlot;
	TempSlot.Item = Item;
	TempSlot.Instance = Instance;

	const int32 Cap = GetEffectiveMaxStackSize(Item);
	int32 freeSpace = 0;

	for (const auto& Slot : Slots)
	{
		if (Slot.IsEmpty())
		{
			freeSpace += Cap;
			continue;
		}
		if (CanStackTogether(Slot, TempSlot))
		{
			freeSpace += Cap - Slot.Quantity;
		}
	}

	return freeSpace;
}


/**
 * Every exit here reports INDEX_NONE, not ToIdx: a move changes the source slot as well as the
 * destination, and the delegate's contract is that an index means "this one slot and nothing
 * else". Naming only ToIdx left the source slot stale for any listener that trusted the index.
 */
bool UInventoryComponent::MoveWithin(int32 FromIdx, int32 ToIdx, int32 Quantity)
{
	if (!Slots.IsValidIndex(FromIdx) || !Slots.IsValidIndex(ToIdx)) return false;
	if (Quantity <= 0 || FromIdx == ToIdx) return false;

	// Merge into existing stack if possible
	if (!Slots[ToIdx].IsEmpty() && CanStackTogether(Slots[FromIdx], Slots[ToIdx]))
	{
		const int32 Space = GetEffectiveMaxStackSize(Slots[ToIdx].Item) - Slots[ToIdx].Quantity;
		const int32 Take = FMath::Min(Quantity, FMath::Min(Slots[FromIdx].Quantity, Space));
		if (Take <= 0) return false;
		Slots[ToIdx].Quantity += Take;
		Slots[FromIdx].Quantity -= Take;
		if (Slots[FromIdx].Quantity <= 0) Slots[FromIdx] = FInventorySlot{};
		OnInventoryChangedType.Broadcast(EInventoryChangeType::ItemMoved, INDEX_NONE);
		return true;
	}

	// Full move: swap source and destination
	if (Quantity >= Slots[FromIdx].Quantity)
	{
		Slots.Swap(FromIdx, ToIdx);
		OnInventoryChangedType.Broadcast(EInventoryChangeType::ItemMoved, INDEX_NONE);
		return true;
	}

	// Split: only into an empty slot
	if (!Slots[ToIdx].IsEmpty()) return false;

	const int32 Take = FMath::Clamp(Quantity, 1, Slots[FromIdx].Quantity - 1);
	FInventorySlot Split;
	Split.Instance = Slots[FromIdx].Instance;
	Split.Quantity = Take;
	Split.Item = Slots[FromIdx].Item;

	Slots[FromIdx].Quantity -= Take;
	Slots[ToIdx] = Split;
	OnInventoryChangedType.Broadcast(EInventoryChangeType::ItemMoved, INDEX_NONE);
	return true;
}

bool UInventoryComponent::TransferTo(UInventoryComponent* To, int32 FromIdx, int32 ToIdx, int32 Quantity)
{
	if (!To || To == this) return false;
	if (!Slots.IsValidIndex(FromIdx) || Quantity <= 0) return false;
	if (!To->Slots.IsValidIndex(ToIdx)) return false;


	const FInventorySlot& Src = Slots[FromIdx];

	// Empty-slot guard before the whitelist test, which dereferences Src.Item. Latent until
	// a container actually set a whitelist - the drying rack is the first one that does.
	if (Src.IsEmpty()) return false;

	if (!To->CategoryWhitelist.IsEmpty() && !To->CategoryWhitelist.Contains(Src.Item->Category)) return false;

	const int32 Want = FMath::Min(Quantity, Src.Quantity);
	if (Want <= 0) return false;

	FInventorySlot Incoming;
	Incoming.Instance = Src.Instance;
	Incoming.Quantity = Want;
	Incoming.Item = Src.Item;

	int32 ActuallyAdded = 0;
	const bool bAdded = To->AddSlotAtIndex(Incoming, ToIdx, ActuallyAdded);

	if (!bAdded || ActuallyAdded <= 0) return false;

	FInventorySlot Removed;
	if (!RemoveAt(FromIdx, ActuallyAdded, Removed))
	{
		return false;
	}

	// RemoveAt above already reported the source side.
	To->OnInventoryChangedType.Broadcast(EInventoryChangeType::ItemAdded, ToIdx);
	return true;
}

bool UInventoryComponent::AddSlotAtIndex(const FInventorySlot& SlotItem, int32 ToIdx, int32& QuantityAdded) {
	if (!Slots.IsValidIndex(ToIdx)) return false;

	if (Slots[ToIdx].IsEmpty())
	{
		// Clamp: this is the path a cross-inventory drag lands on, so an unclamped assign
		// is how a 5-stack would bypass a one-per-slot container (and how any stack could
		// exceed the item's own MaxStackSize).
		QuantityAdded = FMath::Min(SlotItem.Quantity, GetEffectiveMaxStackSize(SlotItem.Item));
		if (QuantityAdded <= 0) return false;

		Slots[ToIdx] = SlotItem;
		Slots[ToIdx].Quantity = QuantityAdded;
		return true;
	}
	else
	{
		if (!CanStackTogether(Slots[ToIdx], SlotItem)) return false;

		const int32 Space = GetEffectiveMaxStackSize(Slots[ToIdx].Item) - Slots[ToIdx].Quantity;
		QuantityAdded = FMath::Clamp(SlotItem.Quantity, 0, Space);
		if (QuantityAdded <= 0) return false;

		Slots[ToIdx].Quantity += QuantityAdded;
		return true;
	}
}

int32 UInventoryComponent::CountNonEmptySlots() {
	int32 NonEmptySlots = 0;

	for (auto& Slot : Slots)
	{
		if (!Slot.IsEmpty())
		{
			NonEmptySlots++;
		}
	}

	return NonEmptySlots;
}

bool UInventoryComponent::TryDropItemToWorld(int32 FromIdx, int32 Quantity)
{
	if (!Slots.IsValidIndex(FromIdx) || Quantity <= 0) return false;

	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World) return false;

	// Nothing to spawn without a WorldItem class, and the slot must stay untouched in that case —
	// removing first would delete the item outright.
	const FInventorySlot& SourceSlot = Slots[FromIdx];
	if (SourceSlot.IsEmpty() || !SourceSlot.Item->WorldItem) return false;

	FInventorySlot outSlot = FInventorySlot{};
	if (!RemoveAt(FromIdx, Quantity, outSlot)) return false;

	// RemoveAt reports what it actually took, which can be less than Quantity.
	const int32 DropCount = outSlot.Quantity;

	// One actor for the whole stack: the world actors carry a Quantity, and picking one up returns
	// all of it. Ten herbs are one pile on the ground, not ten actors fighting the physics solver.
	const FVector SpawnLocation = OwnerActor->GetActorLocation()
		+ OwnerActor->GetActorForwardVector() * DropForwardDistance
		+ FVector(0.f, 0.f, DropHeightOffset);

	FActorSpawnParameters Params;
	Params.Owner = OwnerActor;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(
		outSlot.Item->WorldItem,
		SpawnLocation,
		outSlot.Item->WorldTableRotation,
		Params
	);

	// Never silently eat the items a refused spawn would have carried.
	if (!SpawnedActor)
	{
		AddItem(outSlot.Item.Get(), DropCount, outSlot.Instance);
		return false;
	}

	// Push the definition, instance and stack count onto the spawned actor so picking it back up
	// returns exactly what was dropped. Each world actor type stores them on its own member; there
	// is no shared base holding them, so this dispatch is unavoidable (the workbench does the same).
	if (ABasePlant* SpawnedPlant = Cast<ABasePlant>(SpawnedActor))
	{
		// Builds the harvestable part components and applies WorldScale.
		SpawnedPlant->InitializeFromDefinition(Cast<UPlantItemDefinition>(outSlot.Item.Get()), outSlot.Instance);
		SpawnedPlant->Quantity = DropCount;
		SpawnedPlant->HerbStatus = EHerbStatus::OnStand;
	}
	else if (APlantPart* SpawnedPart = Cast<APlantPart>(SpawnedActor))
	{
		SpawnedPart->SetPartDefinition(Cast<UDataAssetPlantPart>(outSlot.Item.Get()), outSlot.Instance);
		SpawnedPart->Quantity = DropCount;
		SpawnedPart->HerbStatus = EHerbStatus::OnStand;
	}
	else if (ABaseTool* SpawnedTool = Cast<ABaseTool>(SpawnedActor))
	{
		SpawnedTool->Item = const_cast<UToolItemDefinition*>(Cast<UToolItemDefinition>(outSlot.Item.Get()));
		SpawnedTool->Instance = outSlot.Instance;
		SpawnedTool->Quantity = DropCount;
	}
	else if (ABasePotion* SpawnedPotion = Cast<ABasePotion>(SpawnedActor))
	{
		SpawnedPotion->Item = const_cast<UPotionItemDefinition*>(Cast<UPotionItemDefinition>(outSlot.Item.Get()));
		SpawnedPotion->Instance = outSlot.Instance;
		SpawnedPotion->Quantity = DropCount;
	}

	// Let it fall and settle wherever the player is standing. Root-only, matching the plant rule
	// that only the root body simulates and the generated parts ride it.
	if (UPrimitiveComponent* RootPrim = Cast<UPrimitiveComponent>(SpawnedActor->GetRootComponent()))
	{
		RootPrim->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RootPrim->SetSimulatePhysics(true);
	}

	return true;
}

int32 UInventoryComponent::GetEffectiveMaxStackSize(const UItemDefinitionBase* Item) const
{
	if (!Item) return 0;

	// The item proposes, the container caps. bStackable is just "MaxStackSize == 1" said
	// another way, which is what lets the two cases share one path in AddItem.
	const int32 ItemCap = Item->bStackable ? FMath::Max(1, Item->MaxStackSize) : 1;
	return StackSizeLimit > 0 ? FMath::Min(ItemCap, StackSizeLimit) : ItemCap;
}