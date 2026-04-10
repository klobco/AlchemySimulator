// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"
#include "ItemDefinitionBase.h"

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

	UE_LOG(LogTemp, Error, TEXT("Slots: %i"),Slots.Num());
}

bool UInventoryComponent::AddItem(const UItemDefinitionBase* Item, int32 Quantity, const FItemInstanceData& Instance) {

	if (Item == nullptr)
	{
		return false;
	}

	UE_LOG(LogTemp, Error, TEXT("Adding item"));

	if (!Item->bStackable) {
		int32 nonStackableIndex = FindFirstEmptySlotIndex();
		if (nonStackableIndex != -1)
		{
			FInventorySlot slot;

			slot.Instance = Instance;
			slot.Item = Item;
			slot.Quantity = Quantity;

			Slots[nonStackableIndex] = slot;
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Inventory full"));
			return false;
		}
	}
	else
	{
		if (FreeStorageForItem(Item, Instance) >= Quantity)
		{
			int32 ItemsToAdd = Quantity;
			while (true)
			{
				int32 StackSlot = FindFirstStackableSlotIndex(Item, Instance);

				if (StackSlot != -1)
				{
					int32 addNumber = Slots[StackSlot].Item->MaxStackSize - Slots[StackSlot].Quantity;

					if (addNumber < ItemsToAdd)
					{
						Slots[StackSlot].Quantity += addNumber;
						ItemsToAdd -= addNumber;
					}
					else
					{
						Slots[StackSlot].Quantity += ItemsToAdd;
						ItemsToAdd = 0;
						break;
					}

				}
				else {
					break;
				}
			}
			if (ItemsToAdd > 0)
			{
				while (true)
				{
					int32 StackSlot2 = FindFirstEmptySlotIndex();

					if (StackSlot2 != -1)
					{
						int32 addNumber2 = Item->MaxStackSize;

						FInventorySlot slot;

						slot.Instance = Instance;
						slot.Item = Item;

						if (addNumber2 < ItemsToAdd)
						{
							slot.Quantity += addNumber2;
							Slots[StackSlot2] = slot;
							ItemsToAdd -= addNumber2;
						}
						else
						{
							slot.Quantity += ItemsToAdd;
							Slots[StackSlot2] = slot;
							ItemsToAdd = 0;
							break;
						}

					}
					else {
						break;
					}
				}

			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Inventory full"));
			return false;
		}
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveItem(const UItemDefinitionBase* Item, int32 Quantity, const FItemInstanceData& Instance) {
	return true;
}

bool UInventoryComponent::RemoveSlotAtIndex(int32 Index)
{
	if (!Slots.IsValidIndex(Index) || Slots[Index].IsEmpty()) return false;

	Slots[Index] = FInventorySlot();
	OnInventoryChanged.Broadcast();
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
		OnInventoryChanged.Broadcast();
		return true;
	}

	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::RemoveSlotItem(const FInventorySlot Item) {

	if (Item.IsEmpty()) return false;

	for (int32 i = 0; i < Slots.Num(); i++)
	{
		if (Slots[i].Item == Item.Item && Slots[i].Instance.Quality == Item.Instance.Quality)
		{
			Slots[i] = FInventorySlot();
			OnInventoryChanged.Broadcast();
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

	if (A.Item->bStackable && B.Item->bStackable && A.Item == B.Item && A.Instance.Quality == B.Instance.Quality)
	{
		return true;
	}
	return false;
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

	//TODO add instance processing
	if (Item == nullptr || Item->bStackable == false)
	{
		return -1;
	}

	for (int32 Index = 0; Index != Slots.Num(); ++Index)
	{
		if (Slots.IsValidIndex(Index) && Slots[Index].Item == Item && Slots[Index].Quantity < Item->MaxStackSize)
		{
			return Index;
		}
	}

	return -1;
}

int32 UInventoryComponent::FreeStorageForItem(const UItemDefinitionBase* Item, const FItemInstanceData& Instance) const {

	int32 freeSpace = 0;

	if (Item == nullptr)
	{
		return 0;
	}

	for (auto& Slot : Slots)
	{
		if (Slot.IsEmpty())
		{
			freeSpace += Item->MaxStackSize;

			continue;
		}
		if (Slot.Item == Item && Slot.Instance.Quality == Instance.Quality)
		{
			freeSpace += Slot.Item->MaxStackSize - Slot.Quantity;
		}

	}

	AActor* Owner = GetOwner();
	//UE_LOG(LogTemp, Error, TEXT("Free space for item: %i, Owner: %s"), freeSpace, Owner ? *Owner->GetName() : TEXT("NULL"));

	return freeSpace;
}


bool UInventoryComponent::MoveWithin(int32 FromIdx, int32 ToIdx, int32 Quantity)
{
	if (!Slots.IsValidIndex(FromIdx) || Quantity <= 0) return false;

	if (ToIdx < 0) return false;
	if (ToIdx > Slots.Num()) ToIdx = Slots.Num();

	
	if (FromIdx == ToIdx && (Quantity >= Slots[FromIdx].Quantity)) return false;

	FInventorySlot& From = Slots[FromIdx];

	if (Quantity >= From.Quantity)
	{
		FInventorySlot Moving = From;
		Slots.RemoveAt(FromIdx);

		
		if (ToIdx > Slots.Num()) ToIdx = Slots.Num();
		if (FromIdx < ToIdx) ToIdx -= 1;

		Slots.Insert(Moving, ToIdx);
		OnInventoryChanged.Broadcast();
		return true;
	}

	const int32 Take = FMath::Clamp(Quantity, 1, From.Quantity - 1);
	FInventorySlot Split;
	Split.Instance = From.Instance;
	Split.Quantity = Take;
	Split.Item = From.Item;

	From.Quantity -= Take;

	Slots.Insert(Split, FMath::Clamp(ToIdx, 0, Slots.Num()));
	OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::TransferTo(UInventoryComponent* To, int32 FromIdx, int32 ToIdx, int32 Quantity)
{
	if (!To || To == this) return false;
	if (!Slots.IsValidIndex(FromIdx) || Quantity <= 0) return false;
	if (!To->Slots.IsValidIndex(ToIdx)) return false;

	const FInventorySlot& Src = Slots[FromIdx];
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

	To->OnInventoryChanged.Broadcast();
	return true;
}

bool UInventoryComponent::AddSlotAtIndex(const FInventorySlot& SlotItem, int32 ToIdx, int32& QuantityAdded) {
	if (!Slots.IsValidIndex(ToIdx)) return false;

	if (Slots[ToIdx].IsEmpty())
	{
		Slots[ToIdx] = SlotItem;
		QuantityAdded = Slots[ToIdx].Quantity;
		return true;
	}
	else
	{
		if (!CanStackTogether(Slots[ToIdx], SlotItem)) return false;
		if (SlotItem.Quantity + Slots[ToIdx].Quantity > Slots[ToIdx].Item->MaxStackSize)
		{
			QuantityAdded = FMath::Clamp(SlotItem.Quantity, 0, Slots[ToIdx].Item->MaxStackSize- Slots[ToIdx].Quantity);
		}
		else
		{
			QuantityAdded = SlotItem.Quantity;
		}
		Slots[ToIdx].Quantity += QuantityAdded;
		return true;
	}


}