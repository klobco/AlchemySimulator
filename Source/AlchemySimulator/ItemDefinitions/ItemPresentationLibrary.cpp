// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefinitions/ItemPresentationLibrary.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "DataAssets/DataAssetPlantPart.h"
#include "DataAssets/DataAssetProcessingMethod.h"


UTexture2D* UItemPresentationLibrary::ResolveItemIcon(const FItemInstanceData& Instance, const UItemDefinitionBase* ItemDef)
{
    if (!ItemDef)
    {
        return nullptr;
    }

    // Backwards: the last method that sets an icon wins. Two images can't be layered, and a
    // method that overrides nothing (drying) is transparent -- which is what makes the result
    // the same whether you dried then crushed or crushed then dried.
    //
    // A non-empty AppliedMethods is the whole condition. Do not also gate this on
    // Instance.bIsProcessed: that would make the icon disagree with the name and the world mesh,
    // which don't, the moment the two ever drift (see bug-118).
    for (int32 i = Instance.AppliedMethods.Num() - 1; i >= 0; --i)
    {
        if (const UDataAssetProcessingMethod* M = Instance.AppliedMethods[i])
        {
            if (M->Presentation.Icon) return M->Presentation.Icon;
        }
    }

    return ItemDef->Icon;
}

FText UItemPresentationLibrary::ResolveItemDisplayName(const FItemInstanceData& Instance, const UItemDefinitionBase* ItemDef)
{
    if (!ItemDef) return FText::GetEmpty();

    // Forwards, wrapping: each format takes the name so far as {0}, so drying then crushing
    // reads "Crushed Dried Mint Leaf". Names compose; visuals don't.
    FText Name = ItemDef->DisplayName;
    for (const TObjectPtr<const UDataAssetProcessingMethod>& M : Instance.AppliedMethods)
    {
        if (M && !M->Presentation.NameFormat.IsEmpty())
        {
            Name = FText::Format(M->Presentation.NameFormat, Name);
        }
    }
    return Name;
}

void UItemPresentationLibrary::ResolveItemWorldVisuals(const FItemInstanceData& Instance, const UItemDefinitionBase* ItemDef, UStaticMesh*& OutMesh, UMaterialInterface*& OutMaterial, FVector& OutScale)
{
    // Every out-param is written before the first early-out: a caller that passed uninitialised
    // locals must never read one back (bug-117).
    OutMesh = nullptr;
    OutMaterial = nullptr;
    OutScale = ItemDef ? ItemDef->WorldScale : FVector::OneVector;

    if (!ItemDef)
    {
        return;
    }

    // Base layer: the part's own mesh. Items that are not plant parts have no mesh of their own
    // and keep the null set above, but still get the item's WorldScale.
    if (const UDataAssetPlantPart* Part = Cast<UDataAssetPlantPart>(ItemDef))
    {
        OutMesh     = Part->WorldMesh;
        OutMaterial = Part->WorldMaterialOverride;
    }

    for (int32 i = Instance.AppliedMethods.Num() - 1; i >= 0; --i)
    {
        const UDataAssetProcessingMethod* M = Instance.AppliedMethods[i];
        if (M && M->Presentation.WorldMesh)
        {
            // A mesh override replaces the material too, even with null: a leaf's material on
            // a pouch mesh is worse than the pouch's own default.
            OutMesh     = M->Presentation.WorldMesh;
            OutMaterial = M->Presentation.WorldMaterialOverride;
            OutScale    = M->Presentation.WorldScale;
            break;
        }
    }
}
