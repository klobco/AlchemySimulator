// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "Templates/SubclassOf.h"
#include "GameplayTagContainer.h"
#include "DataStructHelpers.h"
#include "ItemMetadata.generated.h"

class UItemDefinitionBase;
class UAlchemyMinigameWidget;
class UDataAssetProcessingMethod;

UENUM(BlueprintType)
enum class EHerbStatus : uint8
{
	Inventory,
	OnStand,
	OnTable
};

/**
 * One thing a tool can do to a target, configured on the tool's UToolItemDefinition.
 *
 * This is the whole reason targets no longer switch on EToolCategory: the tool owns the
 * minigame and the processing method, and the target only answers "do I accept ActionTag?".
 * A new tool is normally a new data asset, not new C++.
 */
USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FToolAction
{
    GENERATED_BODY()

public:
    /** What this action is — ToolAction.Cut, ToolAction.Crush. Targets match against this. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool|Action")
    FGameplayTag ActionTag;

    /** Minigame run to perform the action. An action with no minigame is skipped. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool|Action")
    TSubclassOf<UAlchemyMinigameWidget> MinigameWidgetClass;

    /** Processing applied to the target's instance data. Null for pure-harvest actions (cutting). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tool|Action")
    TObjectPtr<UDataAssetProcessingMethod> ProcessingMethod = nullptr;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FItemInstanceData
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quality = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Freshness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer ProcessingTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ProcessingQuality = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsProcessed = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsPotion = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FGameplayTagContainer RuntimeTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPotionResult PotionResult;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FInventorySlot
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<const UItemDefinitionBase> Item = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
    int32 Quantity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemInstanceData Instance;

    bool IsEmpty() const { return Item == nullptr || Quantity <= 0; }


    
};


USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FItemUseContext
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<AActor> User = nullptr;

    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<class UInventoryComponent> Inventory = nullptr;

    UPROPERTY(BlueprintReadWrite)
    int32 SlotIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadWrite)
    FInventorySlot Slot;
    
    UPROPERTY(BlueprintReadWrite)
    TObjectPtr<AActor> TargetActor = nullptr;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FItemUseResult
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly)
    bool bSuccess = false;

    UPROPERTY(BlueprintReadOnly)
    bool bConsumeOneItem = false;

    UPROPERTY(BlueprintReadOnly)
    FText Message;
};