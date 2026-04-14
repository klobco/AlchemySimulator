// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ItemMetadata.h"
#include "ItemDefinitionBase.h"
#include "InventoryComponent.generated.h"

class UItemDefinitionBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

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

    // Slot helpers
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool CanStackTogether(const FInventorySlot& A, const FInventorySlot& B) const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool IsFull() const;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 FindFirstEmptySlotIndex() const;

    // Event for UI refresh
    UPROPERTY(BlueprintAssignable, Category = "Inventory")
    FOnInventoryChanged OnInventoryChanged;

    UFUNCTION(BlueprintCallable, Category = "Inventory")
    int32 CountNonEmptySlots();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

    int32 FindFirstStackableSlotIndex(const UItemDefinitionBase* Item, const FItemInstanceData& Instance) const;
    int32 FreeStorageForItem(const UItemDefinitionBase* Item, const FItemInstanceData& Instance) const;
};
