// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemMetadata.h"
#include "InventorySlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UInventorySlotWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	FInventorySlot InvSlot;

	UPROPERTY(meta = (BindWidget))
	class UImage* SlotImage = nullptr;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SlotQuantity = nullptr;

	UPROPERTY(BlueprintReadOnly)
	class UInventoryComponent* OwnerInventory;

	UPROPERTY(BlueprintReadOnly)
	int32 SlotIndex = INDEX_NONE;

	UFUNCTION()
	void Setup(FInventorySlot InventorySlot);

	void SetupEmpty();

	virtual FReply NativeOnMouseButtonDown(const FGeometry& MyGeo, const FPointerEvent& MouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& MyGeo, const FPointerEvent& MouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& MyGeo, const FDragDropEvent& DragDropEvent, UDragDropOperation* Operation) override;
	virtual void NativeOnDragEnter(const FGeometry& MyGeo, const FDragDropEvent& DragDropEvent, UDragDropOperation* Operation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& DragDropEvent, UDragDropOperation* Operation) override;
	virtual bool NativeOnDragOver(const FGeometry& MyGeo, const FDragDropEvent& DragDropEvent, UDragDropOperation* Operation) override;
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& Geo, const FPointerEvent& E) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;

protected:

	UFUNCTION()
	void HandleClick();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void Rebuild();

	/**
	 * True between this slot's MouseEnter and MouseLeave. The hover panel is shared by every slot
	 * in every inventory, so only the slot under the pointer may write to it - otherwise a
	 * RebuildGrid elsewhere (which destroys all its slots) would hide a panel it does not own.
	 */
	bool bPointerInside = false;

	/** Shows this slot's current contents in the hover panel, or hides it if the slot is empty. */
	void PushDetails() const;
	void ClearDetails() const;
};
