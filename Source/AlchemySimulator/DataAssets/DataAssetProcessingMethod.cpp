// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/DataAssetProcessingMethod.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "DataAssetProcessingMethod"

#if WITH_EDITOR

EDataValidationResult UDataAssetProcessingMethod::IsDataValid(FDataValidationContext& Context) const
{
    const EDataValidationResult Result = Super::IsDataValid(Context);

    // Warnings, never errors: a half-authored method asset is a normal mid-session state. These
    // exist because every misconfiguration in this system is invisible at runtime -- the resolver
    // just falls back to the item's own icon and name, which looks exactly like "not authored yet".

    const bool bHasAnyOverride =
        Presentation.Icon != nullptr ||
        Presentation.WorldMesh != nullptr ||
        Presentation.WorldMaterialOverride != nullptr ||
        !Presentation.NameFormat.IsEmpty();

    if (!Presentation.WorldMesh)
    {
        // WorldScale and WorldMaterialOverride are only ever read next to a WorldMesh override
        // (UItemPresentationLibrary::ResolveItemWorldVisuals), so without one they are dead data.
        if (!Presentation.WorldScale.Equals(FVector::OneVector))
        {
            Context.AddWarning(FText::Format(LOCTEXT("ScaleWithoutMesh",
                "Presentation.WorldScale is {0} but WorldMesh is unset, so the scale is never applied. "
                "Set a WorldMesh or put the scale back to 1,1,1."),
                FText::FromString(Presentation.WorldScale.ToString())));
        }

        if (Presentation.WorldMaterialOverride)
        {
            Context.AddWarning(LOCTEXT("MaterialWithoutMesh",
                "Presentation.WorldMaterialOverride is set but WorldMesh is unset, so the material is never "
                "applied. A material override only travels with a mesh override."));
        }
    }

    if (!Presentation.NameFormat.IsEmpty() && !Presentation.NameFormat.ToString().Contains(TEXT("{0}")))
    {
        Context.AddWarning(FText::Format(LOCTEXT("NameFormatWithoutArg",
            "Presentation.NameFormat is \"{0}\" but contains no {{0}}, so the item's own name is thrown "
            "away rather than wrapped. Write it as e.g. \"Crushed {{0}}\"."),
            Presentation.NameFormat));
    }

    if (!ProcessingTag.IsValid() && bHasAnyOverride)
    {
        Context.AddWarning(LOCTEXT("PresentationWithoutTag",
            "Presentation has overrides but ProcessingTag is unset. BuildProcessedInstance writes that tag "
            "alongside the method, and CanStackTogether compares tags -- without one, processed items stack "
            "with raw ones and the presentation is unreachable in practice."));
    }

    return Result;
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
