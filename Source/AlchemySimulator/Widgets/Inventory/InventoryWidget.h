// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Inventory/InventoryComponent.h"
#include "InventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	class UUniformGridPanel* Grid_Inventory = nullptr;

	UPROPERTY(meta = (BindWidget))
	class UBorder* SlotBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 NumColumns = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TSubclassOf<class UInventorySlotWidget> EntryWidgetClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	class UInventoryComponent* Inventory = nullptr;

	UFUNCTION(BlueprintCallable)
	void SetInventory(UInventoryComponent* InInv);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	int32 CapacitySlots;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RebuildGrid();

	/**
	 * Re-run Setup on the one slot widget that changed, instead of rebuilding the grid.
	 *
	 * Not polish: RebuildGrid clears the panel and CreateWidgets every slot again, and a running
	 * drying rack reports a change every game minute -- ten times a second at the default clock
	 * speed. The index on OnInventoryChangedType exists precisely so this is possible.
	 */
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RefreshSlot(int32 SlotIndex);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	bool bIsRebuilding = false;

	/** The widgets RebuildGrid created, index-aligned with the inventory's slots. */
	UPROPERTY(Transient)
	TArray<class UInventorySlotWidget*> SlotWidgets;


	UFUNCTION()
	void HandleInventoryChanged(EInventoryChangeType ChangeType, int32 SlotIndex);

};
