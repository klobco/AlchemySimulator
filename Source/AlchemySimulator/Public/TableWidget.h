// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameWidget.h"
#include "ActionTabWidget.h"
#include "TableWidget.generated.h"

/**
 * Widget for the workbench action/tool management menu.
 * Pushed onto the modal stack by ABasicInteractableStationObject.
 */
UCLASS()
class ALCHEMYSIMULATOR_API UTableWidget : public UBaseGameWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget)) class UVerticalBox* ActionItems = nullptr;
	UPROPERTY(meta = (BindWidget)) class UVerticalBox* InventoryItems = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "widgets")
	TSubclassOf<class UActionTabItemWidget> EntryItemWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "widgets")
	TSubclassOf<class UActionTabWidget> EntryActionWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	class UInventoryComponent* Inventory = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "actions")
	EAction SelectedAction = EAction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "items")
	class ABasicInteractableStationObject* Table = nullptr;

	UFUNCTION()
	void SetAction(EAction Act);

	UFUNCTION()
	void SetTable(ABasicInteractableStationObject* InTable);

	UFUNCTION()
	void ItemAction(FInventorySlot InSlot);

	UFUNCTION(BlueprintCallable)
	void Setup(UInventoryComponent* InInventory);

	UFUNCTION(BlueprintCallable)
	void Rebuild();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void OnClosed_Implementation() override;
};
