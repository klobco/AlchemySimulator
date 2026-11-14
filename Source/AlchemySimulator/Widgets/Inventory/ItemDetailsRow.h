// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemDetailsRow.generated.h"

/**
 * One "Name: Value" line in the item details panel. Created and filled by
 * UInventorySlotDetailsWidget::Setup — WBP_DetailsRow only lays out the two text blocks.
 */
UCLASS()
class ALCHEMYSIMULATOR_API UItemDetailsRow : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* NameText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ValueText;

	UFUNCTION(BlueprintCallable, Category = "Item Details")
	void SetRow(const FText& InName, const FText& InValue);
};
