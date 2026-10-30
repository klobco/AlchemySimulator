// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemMetadata.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "InventoryComponent.generated.h"

class UItemDefinitionBase;

UENUM(BlueprintType)
enum class EInventoryChangeType : uint8
{
    ItemAdded   UMETA(DisplayName = "Item Added"),
    ItemRemoved UMETA(DisplayName = "Item Removed"),
    ItemMoved  UMETA(DisplayName = "Item Moved"),
    ItemUpdated UMETA(DisplayName = "Item Updated")
};


/**
 * The inventory changed. SlotIndex is the slot that changed, or INDEX_NONE when one
 * operation touched several slots (AddItem spilling across stacks) and the listener
 * should treat it as a full refresh.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FOnInventoryChangedType,
    EInventoryChangeType, ChangeType,
    int32, SlotIndex
);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ALCHEMYSIMULATOR_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<FInventorySlot> Slots;

	UInventoryComponent();

    // Inventory configuration
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    int32 MaxSlots = 24;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
    TArray<EItemCategory> CategoryWhitelist;

    /**
     * Per-container cap on how many of one item share a slot. 0 = no cap, the item's own
     * MaxStackSize applies. 1 = this container never stacks (one item per slot) - what a
     * drying rack wants, while the same herb still stacks in the player's bag.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Stacking", meta = (ClampMin = "0"))
    int32 StackSizeLimit = 0;

    // Where TryDropItemToWorld puts the spawned actor, relative to the owner.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Drop")
    float DropForwardDistance = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Drop")
    float DropHeightOffset = 20.f;

    /** Sideways spacing between the actors of a dropped stack, so they do not spawn inside each other. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory|Drop")
    float DropSideStep = 25.f;

    // Read-only access (UI)
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    const TArray<FInventorySlot>& GetSlots() const { return Slots; }

    // Main operations
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddItem(const UItemDefinitionBase* Item, int32 Quantity, const FItemInstanceData& Instance);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool AddSlotAtIndex(const FInventorySlot& SlotItem, int32 ToIdx ,int32& QuantityAdded);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveItem(const UItemDefinitionBase* Item, int32 Quantity, const FItemInstanceData& Instance);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveSlotItem(const FInventorySlot Item);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveSlotAtIndex(int32 Index);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool RemoveAt(int32 FromIdx, int32 Quantity, FInventorySlot& OutRemoved);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool HasItem(const UItemDefinitionBase* Item, int32 Quantity, const FItemInstanceData& Instance) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 CountItem(const UItemDefinitionBase* Item, const FItemInstanceData& Instance) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TransferTo(UInventoryComponent* To, int32 FromIdx,int32 ToIdx, int32 Quantity);

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool MoveWithin(int32 FromIdx, int32 ToIdx, int32 Quantity);

    /**
     * Overwrite one slot's per-instance data, leaving its definition and quantity alone, and
     * report that single slot as ItemUpdated.
     *
     * The only way an item is mutated in place -- drying is the first thing that does it, and
     * routing it through here is what keeps anything from writing Slots[i].Instance behind the
     * delegate's back. Returns false for an out-of-range or empty slot.
     */
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool UpdateSlotInstance(int32 Index, const FItemInstanceData& Instance);

    /**
     * The real stack cap for Item in THIS container: the lower of the item's own MaxStackSize
     * and StackSizeLimit. >= 1 for a valid item, 0 for null. Every stacking decision goes
     * through here instead of reading Item->MaxStackSize directly.
     */
    UFUNCTION(BlueprintPure, Category = "Inventory|Stacking")
    int32 GetEffectiveMaxStackSize(const UItemDefinitionBase* Item) const;

    // Slot helpers
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool CanStackTogether(const FInventorySlot& A, const FInventorySlot& B) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool IsFull() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 FindFirstEmptySlotIndex() const;

    // Event for UI refresh
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChangedType OnInventoryChangedType;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 CountNonEmptySlots();

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool TryDropItemToWorld(int32 FromIdx, int32 Quantity);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    int32 FindFirstStackableSlotIndex(const UItemDefinitionBase* Item, const FItemInstanceData& Instance) const;
    int32 FreeStorageForItem(const UItemDefinitionBase* Item, const FItemInstanceData& Instance) const;
};
