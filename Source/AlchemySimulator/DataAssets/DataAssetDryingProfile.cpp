// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/DataAssetDryingProfile.h"

#include "ItemDefinitions/ItemDefinitionBase.h"
#include "Tags/AlchemyGameplayTags.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "DataAssetDryingProfile"

int32 UDataAssetDryingProfile::GetStageIndexForNormalized(float Normalized) const
{
    if (Stages.IsEmpty())
    {
        return INDEX_NONE;
    }

    // Linear scan: five rows, and the ascending order it relies on is what IsDataValid warns
    // about. Falls back to row 0 for a below-the-first-threshold value, so a profile whose row 0
    // was mis-authored above 0.0 still reports a stage rather than nothing.
    int32 Result = 0;
    for (int32 i = 0; i < Stages.Num(); ++i)
    {
        if (Normalized >= Stages[i].EnterAtNormalized)
        {
            Result = i;
        }
        else
        {
            break;
        }
    }
    return Result;
}

float UDataAssetDryingProfile::GetMaxNormalized() const
{
    return Stages.IsEmpty() ? 0.f : Stages.Last().EnterAtNormalized;
}

bool UDataAssetDryingProfile::ResolveStageFor(const FItemInstanceData& Instance, const UItemDefinitionBase* ItemDef,
                                              int32& OutStageIndex, float& OutProgress01)
{
    // Written before the first early-out: a caller that passed uninitialised locals must never
    // read one back (bug-117).
    OutStageIndex = INDEX_NONE;
    OutProgress01 = 0.f;

    if (!ItemDef)
    {
        return false;
    }

    const UDataAssetDryingProfile* Profile = ItemDef->DryingProfile;
    if (!Profile || Profile->Stages.IsEmpty() || ItemDef->MinutesToFullyDry <= 0.f)
    {
        return false;
    }

    const float Normalized = Instance.DryingMinutes / ItemDef->MinutesToFullyDry;
    const float MaxNormalized = Profile->GetMaxNormalized();

    OutStageIndex = Profile->GetStageIndexForNormalized(Normalized);
    OutProgress01 = MaxNormalized > 0.f ? FMath::Clamp(Normalized / MaxNormalized, 0.f, 1.f) : 0.f;

    return OutStageIndex != INDEX_NONE;
}

#if WITH_EDITOR

namespace
{
    /** Does this row say anything about how the item looks or reads? */
    bool PresentationIsSet(const FProcessedPresentation& Presentation)
    {
        return Presentation.Icon != nullptr
            || Presentation.WorldMesh != nullptr
            || !Presentation.NameFormat.IsEmpty();
    }
}

EDataValidationResult UDataAssetDryingProfile::IsDataValid(FDataValidationContext& Context) const
{
    const EDataValidationResult Result = Super::IsDataValid(Context);

    // Warnings, not errors, like every other asset in this system: a half-authored curve is a
    // normal mid-session state. They exist because all of this fails silently at runtime -- a
    // broken profile just means nothing ever dries, with no log anywhere.

    if (Stages.Num() < 2)
    {
        Context.AddWarning(LOCTEXT("TooFewStages",
            "A drying profile needs at least two stages (a starting one and something to dry into). "
            "With fewer, nothing on a rack will ever change state."));
    }

    if (!Stages.IsEmpty() && !FMath::IsNearlyZero(Stages[0].EnterAtNormalized))
    {
        Context.AddWarning(FText::Format(LOCTEXT("FirstStageNotZero",
            "Stage 0 enters at {0} rather than 0.0. A freshly hung item is below every threshold, "
            "so it falls back to stage 0 anyway -- set it to 0.0 and say so."),
            FText::AsNumber(Stages[0].EnterAtNormalized)));
    }

    for (int32 i = 0; i < Stages.Num(); ++i)
    {
        const FDryingStage& Stage = Stages[i];

        if (i > 0 && Stage.EnterAtNormalized <= Stages[i - 1].EnterAtNormalized)
        {
            Context.AddWarning(FText::Format(LOCTEXT("ThresholdsNotAscending",
                "Stage {0} enters at {1}, which is not above stage {2}'s {3}. Thresholds must ascend "
                "or the stage lookup will skip this row entirely."),
                FText::AsNumber(i), FText::AsNumber(Stage.EnterAtNormalized),
                FText::AsNumber(i - 1), FText::AsNumber(Stages[i - 1].EnterAtNormalized)));
        }

        if (!Stage.StageTag.IsValid())
        {
            Context.AddWarning(FText::Format(LOCTEXT("NoStageTag",
                "Stage {0} has no StageTag, so OnDryingStageChanged will broadcast an empty tag for it."),
                FText::AsNumber(i)));
        }
        else
        {
            // MatchesTag is hierarchy-aware, so this keeps working as new children are added.
            // The root is a native tag and cannot be missing.
            if (!Stage.StageTag.MatchesTag(TAG_Drying))
            {
                Context.AddWarning(FText::Format(LOCTEXT("BadStageTag",
                    "Stage {0}'s StageTag is '{1}', which is not a Drying.* tag. The drying stage is derived "
                    "state and has its own family -- a Processing.* tag here would be a second owner of it."),
                    FText::AsNumber(i), FText::FromName(Stage.StageTag.GetTagName())));
            }

            for (int32 j = 0; j < i; ++j)
            {
                if (Stages[j].StageTag == Stage.StageTag)
                {
                    Context.AddWarning(FText::Format(LOCTEXT("DuplicateStageTag",
                        "Stages {0} and {1} share the StageTag '{2}'. Listeners cannot tell the two bands apart."),
                        FText::AsNumber(j), FText::AsNumber(i), FText::FromName(Stage.StageTag.GetTagName())));
                    break;
                }
            }
        }

        if (!Stage.Presentation.NameFormat.IsEmpty() &&
            !Stage.Presentation.NameFormat.ToString().Contains(TEXT("{0}")))
        {
            Context.AddWarning(FText::Format(LOCTEXT("NameFormatNoPlaceholder",
                "Stage {0}'s NameFormat has no {{0}}, so it replaces the item's name instead of wrapping it "
                "-- every item in this band would read the same."),
                FText::AsNumber(i)));
        }

        if (Stage.CommitMethod && PresentationIsSet(Stage.Presentation))
        {
            Context.AddWarning(FText::Format(LOCTEXT("StageCommitsAndPresents",
                "Stage {0} has both a CommitMethod and its own Presentation. Both layers apply, so the item "
                "ends up reading like \"Dried Dry Mint Leaf\". Let the method own the name and look of a "
                "band it commits, and leave the stage's Presentation empty."),
                FText::AsNumber(i)));
        }
    }

    return Result;
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
