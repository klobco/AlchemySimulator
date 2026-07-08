// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	bool bIsRebuilding = false;


	UFUNCTION()
	void HandleInventoryChanged();

};
