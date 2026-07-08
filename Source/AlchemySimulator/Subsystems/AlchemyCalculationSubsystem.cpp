// Fill out your copyright notice in the Description page of Project Settings.

#include "DataAssets/DataAssetPlantPart.h"
#include "DataAssets/DataAssetSubstanceDefinition.h"
#include "DataAssets/DataAssetAlchemyEfectDefinition.h"
#include "GameplayTagContainer.h"
#include "Subsystems/AlchemyCalculationSubsystem.h"

FPotionResult UAlchemyCalculationSubsystem::CalculatePotionFromSlots(
    const TArray<FInventorySlot>& IngredientSlots
) const
{
    FPotionResult Result;

    FLinearColor MixedColor = FLinearColor::Black;
    float ColorWeight = 0.0f;

    int32 ValidIngredientCount = 0;

    for (const FInventorySlot& Slot : IngredientSlots)
    {
        if (Slot.IsEmpty() || !Slot.Item)
        {
            continue;
        }

        const UDataAssetPlantPart* PlantPart = Cast<UDataAssetPlantPart>(Slot.Item);

        if (!PlantPart)
        {
            UE_LOG(LogTemp, Warning, TEXT("[AlchemyCalculation] Item is not a plant part. Skipping."));
            continue;
        }

        ValidIngredientCount++;

        const FItemInstanceData& Instance = Slot.Instance;

        const float QualityMultiplier = FMath::Clamp(Instance.Quality / 100.0f, 0.0f, 2.0f);
        const float FreshnessMultiplier = FMath::Clamp(Instance.Freshness, 0.0f, 2.0f);
        const float ProcessingQualityMultiplier = FMath::Clamp(Instance.ProcessingQuality, 0.0f, 2.0f);

        for (const FSubstanceAmount& SubstanceAmount : PlantPart->Substances)
        {
            UDataAssetSubstanceDefinition* Substance = SubstanceAmount.Substance;

            if (!Substance)
            {
                continue;
            }

            float FinalAmount = SubstanceAmount.Amount;

            FinalAmount *= PlantPart->BaseQuality;
            FinalAmount *= QualityMultiplier;
            FinalAmount *= FreshnessMultiplier;
            FinalAmount *= ProcessingQualityMultiplier;
            FinalAmount *= GetProcessingMultiplier(Instance, Substance);

            if (FinalAmount <= 0.0f)
            {
                continue;
            }

            for (const FAlchemyEffectValue& BaseEffect : Substance->BaseEffects)
            {
                if (!BaseEffect.Effect)
                {
                    continue;
                }

                const float FinalEffectValue = GetEffectValueSafe(BaseEffect.Value, FinalAmount);

                AddEffectValue(
                    Result.FinalEffects,
                    BaseEffect.Effect,
                    FinalEffectValue
                );

                if (BaseEffect.Effect->bIsSideEffect)
                {
                    Result.Toxicity += FMath::Max(0.0f, FinalEffectValue);
                }
            }

            Result.Toxicity += Substance->BaseToxicity * FinalAmount / 10.0f;

            MixedColor += Substance->ColorHint * FinalAmount;
            ColorWeight += FinalAmount;
        }
    }

    if (ColorWeight > 0.0f)
    {
        Result.PotionColor = MixedColor / ColorWeight;
    }

    Result.Toxicity = FMath::Max(0.0f, Result.Toxicity);

    Result.Purity = FMath::Clamp(
        1.0f - Result.Toxicity / 100.0f,
        0.0f,
        1.0f
    );

    Result.Stability = FMath::Clamp(
        1.0f - ValidIngredientCount * 0.05f,
        0.2f,
        1.0f
    );

    return Result;
}

void UAlchemyCalculationSubsystem::AddEffectValue(
    TArray<FAlchemyEffectValue>& Effects,
    UDataAssetAlchemyEfectDefinition* Effect,
    float Value
) const
{
    if (!Effect || FMath::IsNearlyZero(Value))
    {
        return;
    }

    for (FAlchemyEffectValue& ExistingEffect : Effects)
    {
        if (ExistingEffect.Effect == Effect)
        {
            ExistingEffect.Value += Value;
            return;
        }
    }

    FAlchemyEffectValue NewEffect;
    NewEffect.Effect = Effect;
    NewEffect.Value = Value;

    Effects.Add(NewEffect);
}

float UAlchemyCalculationSubsystem::GetProcessingMultiplier(
    const FItemInstanceData& Instance,
    const UDataAssetSubstanceDefinition* Substance
) const
{
    if (!Substance)
    {
        return 1.0f;
    }

    float Multiplier = 1.0f;

    const FGameplayTag DriedTag =
        FGameplayTag::RequestGameplayTag(TEXT("Processing.Dried"), false);

    const FGameplayTag BoiledTag =
        FGameplayTag::RequestGameplayTag(TEXT("Processing.Boiled"), false);

    const FGameplayTag CrushedTag =
        FGameplayTag::RequestGameplayTag(TEXT("Processing.Crushed"), false);

    if (DriedTag.IsValid() && Instance.ProcessingTags.HasTagExact(DriedTag))
    {
        Multiplier *= Substance->DryingStability;
    }

    if (BoiledTag.IsValid() && Instance.ProcessingTags.HasTagExact(BoiledTag))
    {
        Multiplier *= Substance->HeatStability;
    }

    if (CrushedTag.IsValid() && Instance.ProcessingTags.HasTagExact(CrushedTag))
    {
        Multiplier *= Substance->CrushingStability;
    }

    return FMath::Clamp(Multiplier, 0.0f, 3.0f);
}

float UAlchemyCalculationSubsystem::GetEffectValueSafe(
    float BaseValue,
    float FinalAmount
) const
{
    return BaseValue * FinalAmount / 10.0f;
}