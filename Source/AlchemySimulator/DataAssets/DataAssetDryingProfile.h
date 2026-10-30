// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DataAssets/DataAssetProcessingMethod.h"
#include "ItemMetadata.h"
#include "DataAssetDryingProfile.generated.h"

class UItemDefinitionBase;

/**
 * One band of the drying curve.
 *
 * Thresholds are NORMALISED (0..1 of the item's MinutesToFullyDry), so one shared profile asset
 * describes the shape of drying for every herb in the game and each part only authors how long
 * its own drying takes. Same shared-generic shape as one BP_Plant per species.
 */
USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FDryingStage
{
    GENERATED_BODY()

public:
    /** Drying.Moist, Drying.Dry, ... Derived state: never written onto ProcessingTags. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drying")
    FGameplayTag StageTag;

    /** Where this band starts, as a fraction of MinutesToFullyDry. Ascending; row 0 must be 0. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drying", meta = (ClampMin = "0.0"))
    float EnterAtNormalized = 0.f;

    /**
     * How the item reads and looks while it is in this band.
     *
     * Reusing FProcessedPresentation is the point: NameFormat ("Moist {0}"), Icon and the world
     * mesh trio already resolve through UItemPresentationLibrary, so a stage gets a name and a
     * look with no new presentation code.
     *
     * Leave it empty on any row that has a CommitMethod -- the method owns the name there, or the
     * item reads "Dried Dry Mint Leaf" the moment it commits. IsDataValid warns about it.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drying")
    FProcessedPresentation Presentation;

    /**
     * Processing committed to the item when it crosses INTO this band, which is what turns a
     * derived stage into permanent state. Null for the in-between bands, which are presentation
     * only.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drying")
    TObjectPtr<UDataAssetProcessingMethod> CommitMethod = nullptr;
};

/**
 * The stage curve a drying item follows: Wet -> Moist -> Semi-Dry -> Dry -> Scorched.
 *
 * Shared across items; the per-item knob is UItemDefinitionBase::MinutesToFullyDry. Every
 * consumer (the rack, the inventory's stacking test, the presentation library, the slot's
 * progress bar) resolves a stage through ResolveStageFor rather than dividing for itself.
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDataAssetDryingProfile : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drying")
    TArray<FDryingStage> Stages;

    /** Index of the last band whose EnterAtNormalized <= Normalized. INDEX_NONE when empty. */
    UFUNCTION(BlueprintPure, Category = "Drying")
    int32 GetStageIndexForNormalized(float Normalized) const;

    /** The last band's threshold -- where drying stops accumulating. 0 when empty. */
    UFUNCTION(BlueprintPure, Category = "Drying")
    float GetMaxNormalized() const;

    /**
     * The composite lookup: finds the profile and the duration on the item definition, divides,
     * and answers which band the instance is in plus a 0..1 bar value (1.0 = the last band's
     * threshold, i.e. fully scorched, not fully dry).
     *
     * Returns false when the item cannot be dried at all -- no definition, no profile, no
     * duration, or an empty curve. Both out-params are written before every early-out, so a
     * caller may pass uninitialised locals (bug-117).
     */
    UFUNCTION(BlueprintPure, Category = "Drying")
    static bool ResolveStageFor(const FItemInstanceData& Instance, const UItemDefinitionBase* ItemDef,
                                int32& OutStageIndex, float& OutProgress01);

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};
