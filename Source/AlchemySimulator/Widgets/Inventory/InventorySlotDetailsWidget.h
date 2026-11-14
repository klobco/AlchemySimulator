// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemMetadata.h"
#include "InventorySlotDetailsWidget.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UInventorySlotDetailsWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* NameText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DescriptionText;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* IconImage;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* DetailsBox;

	/** Row widget (WBP_DetailsRow) spawned into DetailsBox, one per detail line. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Details")
	TSubclassOf<class UItemDetailsRow> DetailsRowClass;

	UFUNCTION()
	void Setup(const class UItemDefinitionBase* ItemDef,const FItemInstanceData& ItemData);

protected:
	/** Creates one row and appends it to DetailsBox. */
	void AddDetailRow(const FText& Name, const FText& Value);
};
