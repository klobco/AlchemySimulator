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
    /** The actor spawned for this item in the world. Shared/generic, not per-species. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|World")
    TSubclassOf<class AActor> WorldItem = nullptr;

    /** Actor scale when spawned. A scale that suits a sprig of mint will not suit a tree stem. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|World")
    FVector WorldScale = FVector::OneVector;

    /** Rotation applied when the item is placed on a workbench table. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|World")
    FRotator WorldTableRotation = FRotator::ZeroRotator;

    // --- Categorization ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Category")
    EItemCategory Category = EItemCategory::None;

    // --- Stacking ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stacking")
    bool bStackable = true;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Stacking", meta = (ClampMin = "1", EditCondition = "bStackable"))
    int32 MaxStackSize = 99;

    // --- Drying ---
    /**
     * The stage curve this item follows on a drying rack. Null = this item cannot be dried,
     * which is what every drying consumer tests first.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Drying")
    TObjectPtr<class UDataAssetDryingProfile> DryingProfile = nullptr;

    /**
     * How long this item takes to reach the profile's "fully dry" point, in game minutes. The
     * profile's thresholds are normalised against this, so a leaf and a root share one curve and
     * differ only here. <= 0 = cannot be dried.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item|Drying", meta = (ClampMin = "0.0"))
    float MinutesToFullyDry = 0.f;

    /**
     * May this item be put into the processing state Tag names?
     *
     * The one question every route into a processing state asks: a tool action asks it through
     * UDataAssetPlantPart::AcceptsToolAction, and the drying rack asks it directly. The base
     * answer is yes for everything -- only assets that actually restrict processing override it,
     * and overriding it is what keeps the tool route and the rack route from drifting apart.
     */
    UFUNCTION(BlueprintCallable, Category = "Item|Processing")
    virtual bool AcceptsProcessingTag(const FGameplayTag& Tag) const { return true; }

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
