// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemMetadata.h"
#include "ItemDefinitionBase.h"
#include "ActionTabItemWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemButton, FInventorySlot, ItemSlot);

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UActionTabItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (ExposeOnSpawn = true))
	FInventorySlot ItemSlot;

	UPROPERTY(meta = (BindWidget)) class UTextBlock* ItemName = nullptr;
	UPROPERTY(meta = (BindWidget)) class UButton* ItemButton = nullptr;
	UPROPERTY(meta = (BindWidget)) class UImage* ItemIcon = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "action")
	FOnItemButton ItemClicked;

	UFUNCTION()
	void HandleActionClicked();

	UFUNCTION(BlueprintCallable)
	void Setup(const FInventorySlot& InStack);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void Refresh();
};
