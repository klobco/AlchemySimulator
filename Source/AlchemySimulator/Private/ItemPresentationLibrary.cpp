// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemPresentationLibrary.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "DataAssets/DataAssetPlantPart.h"
#include "DataAssets/DataAssetProcessingMethod.h"


UTexture2D* UItemPresentationLibrary::ResolveProcessedIcon(const FItemInstanceData& Instance, const UItemDefinitionBase* ItemDef)
{
    if (!ItemDef)
    {
        return nullptr;
    }

    if (Instance.bIsProcessed)
    {
        for (int32 i = Instance.AppliedMethods.Num() - 1; i >= 0; --i)
        {
            if (const UDataAssetProcessingMethod* M = Instance.AppliedMethods[i])
            {
                if (M->Presentation.Icon) return M->Presentation.Icon;
            }
        }
    }

    return ItemDef->Icon;
}

FText UItemPresentationLibrary::ResolveItemDisplayName(const FItemInstanceData& Instance, const UItemDefinitionBase* ItemDef)
{
    if (!ItemDef) return FText::GetEmpty();

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

void UItemPresentationLibrary::ResolveItemWorldVisual(const FItemInstanceData& Instance, const UItemDefinitionBase* ItemDef, UStaticMesh*& OutMesh, UMaterialInterface*& OutMaterial, FVector& OutScale)
{
    if (!ItemDef)
    {
        OutMesh = nullptr;
        OutMaterial = nullptr;
        OutScale = FVector::OneVector;
        return;
    }

    if (const UDataAssetPlantPart* Part = Cast<UDataAssetPlantPart>(ItemDef))
    {
        OutMesh     = Part->WorldMesh;
        OutMaterial = Part->WorldMaterialOverride;
        OutScale    = ItemDef ? ItemDef->WorldScale : FVector::OneVector;
    }

    for (int32 i = Instance.AppliedMethods.Num() - 1; i >= 0; --i)
    {
        const UDataAssetProcessingMethod* M = Instance.AppliedMethods[i];
        if (M && M->Presentation.WorldMesh)
        {
            OutMesh     = M->Presentation.WorldMesh;
            OutMaterial = M->Presentation.WorldMaterialOverride;
            OutScale    = M->Presentation.WorldScale;
            break;
        }
    }
}