// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DataAssetProcessingMethod.generated.h"


class UDataAssetAlchemyEfectDefinition;
class UDataAssetSubstanceDefinition;

USTRUCT(BlueprintType)
struct FProcessingEffectModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UDataAssetAlchemyEfectDefinition> Effect = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Multiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float AdditiveBonus = 0.0f;
};

USTRUCT(BlueprintType)
struct FSubstanceTransformationRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UDataAssetSubstanceDefinition> FromSubstance = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<UDataAssetSubstanceDefinition> ToSubstance = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float Chance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    float AmountMultiplier = 1.0f;
};

/** How an item looks and reads once this method has been applied to it.
 *
 *  Every field is independently opt-in: null/empty means "this method does not change
 *  that one thing". Name and appearance are separate axes on purpose -- drying renames
 *  a leaf to "Dried Mint Leaf" while leaving it looking exactly like a leaf, and a
 *  method may just as well change the look without touching the name. */
USTRUCT(BlueprintType)
struct FProcessedPresentation
{
    GENERATED_BODY()

    /** Inventory icon. Null keeps the item's own Icon. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
    TObjectPtr<UTexture2D> Icon = nullptr;

    /** World mesh when the item is dropped or placed. Null keeps the part's own WorldMesh. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
    TObjectPtr<UStaticMesh> WorldMesh = nullptr;

    /** Applied only when WorldMesh is set -- a mesh override replaces the material too. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
    TObjectPtr<UMaterialInterface> WorldMaterialOverride = nullptr;

    /** Applied only when WorldMesh is set -- a pouch is not a leaf's size. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation",
              meta = (EditCondition = "WorldMesh != nullptr"))
    FVector WorldScale = FVector::OneVector;

    /** Wraps the item's name; {0} is the name so far. e.g. "Crushed {0}". Empty keeps it. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
    FText NameFormat;
};


/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDataAssetProcessingMethod : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText DisplayName;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag ProcessingTag;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float GeneralPotencyMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float ToxicityMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    float StabilityMultiplier = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FProcessingEffectModifier> EffectModifiers;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FSubstanceTransformationRule> TransformationRules;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Presentation")
    FProcessedPresentation Presentation;

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
