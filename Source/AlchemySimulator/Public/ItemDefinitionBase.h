// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemDefinitionBase.generated.h"

class UTexture2D;
class UStaticMesh;


UENUM(BlueprintType)
enum class EItemCategory : uint8
{
    None,
    Herb,
    Ingredient,
    Tool,
    Knife,
    Potion
};

UENUM(BlueprintType)
enum class EIngredientPart : uint8
{
    None,
    Whole,
    Leaf,
    Stem,
    Root,
    Fruit,
    Flower
};

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UItemDefinitionBase : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        static const FPrimaryAssetType Type = TEXT("Item");
        return FPrimaryAssetId(Type, GetFName());
    }

    // --- Identification / UI ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Identity")
    FName ItemId;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
    FText Description;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|UI")
    TObjectPtr<UTexture2D> Icon = nullptr;

    // --- World representation (optional) ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|World")
    TSubclassOf<class ABaseTool> WorldItem = nullptr;

    // --- Workbench cursor ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Workbench")
    TObjectPtr<UTexture2D> WorkbenchCursor = nullptr;

    // --- Categorization ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Category")
    EItemCategory Category = EItemCategory::None;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Category")
    EIngredientPart Part = EIngredientPart::None;

    // Gameplay tags for filtering, crafting rules, effects, tools, etc.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Tags")
    FGameplayTagContainer Tags;

    // --- Stacking ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stacking")
    bool bStackable = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stacking", meta = (ClampMin = "1", EditCondition = "bStackable"))
    int32 MaxStackSize = 99;

    // --- Economy ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Economy", meta = (ClampMin = "0"))
    int32 BaseValue = 0;

    // --- Future-ready: optional per-item limits ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Misc", meta = (ClampMin = "0"))
    float Weight = 0.0f;
};
