// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "ItemMetadata.h"
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
    Potion
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
    TSubclassOf<class AActor> WorldItem = nullptr;

    // --- Categorization ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Category")
    EItemCategory Category = EItemCategory::None;

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

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Item|Use")
    bool CanUseItem(const FItemUseContext& Context) const;

    virtual bool CanUseItem_Implementation(const FItemUseContext& Context) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Item|Use")
    FItemUseResult UseItem(const FItemUseContext& Context) const;

    virtual FItemUseResult UseItem_Implementation(const FItemUseContext& Context) const;
};
