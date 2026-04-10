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

	UPROPERTY(meta = (BindWidget))
	class UButton* SlotButton = nullptr;


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

protected:

	UFUNCTION()
	void HandleClick();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void Rebuild();
};
