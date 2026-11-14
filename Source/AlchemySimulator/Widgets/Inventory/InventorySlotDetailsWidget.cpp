// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotDetailsWidget.h"
#include "AlchemySimulator.h"
#include "ItemDetailsRow.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "ItemDefinitions/ItemPresentationLibrary.h"
#include "DataAssets/DataAssetDryingProfile.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/VerticalBox.h"

#define LOCTEXT_NAMESPACE "InventorySlotDetails"

namespace
{
    /** "Processing.Crushed" -> "Crushed". Display-only; never parse tags back from this. */
    FString TagLeaf(const FGameplayTag& Tag)
    {
        FString Leaf = Tag.ToString();
        int32 DotIndex;
        if (Leaf.FindLastChar(TEXT('.'), DotIndex))
        {
            Leaf.RightChopInline(DotIndex + 1);
        }
        return Leaf;
    }
}

void UInventorySlotDetailsWidget::AddDetailRow(const FText& Name, const FText& Value)
{
    if (!DetailsBox || !DetailsRowClass)
    {
        return;
    }

    UItemDetailsRow* Row = CreateWidget<UItemDetailsRow>(this, DetailsRowClass);
    if (!Row)
    {
        return;
    }

    Row->SetRow(Name, Value);
    DetailsBox->AddChildToVerticalBox(Row);
}

void UInventorySlotDetailsWidget::Setup(const UItemDefinitionBase* ItemDef, const FItemInstanceData& ItemData)
{
    // The same widget instance is reused for every hovered slot, so old rows must go first.
    if (DetailsBox)
    {
        DetailsBox->ClearChildren();
    }

    if (!ItemDef)
    {
        return;
    }

    // Name and icon resolve through the presentation library, the same as the slot and the drag
    // visual, so a crushed or moist part reads the same here as it does in the grid.
    NameText->SetText(UItemPresentationLibrary::ResolveItemDisplayName(ItemData, ItemDef));
    DescriptionText->SetText(ItemDef->Description);
    IconImage->SetBrushFromTexture(UItemPresentationLibrary::ResolveItemIcon(ItemData, ItemDef));

    if (!DetailsRowClass)
    {
        UE_LOG(LogAlchemySimulator, Warning,
            TEXT("[ItemDetails] DetailsRowClass is unset on %s - no detail rows will show."), *GetNameSafe(GetClass()));
        return;
    }

    AddDetailRow(LOCTEXT("Quality", "Quality"), FText::AsNumber(ItemData.Quality));

    if (ItemDef->Category == EItemCategory::Herb || ItemDef->Category == EItemCategory::Ingredient)
    {
        AddDetailRow(LOCTEXT("Freshness", "Freshness"), FText::AsPercent(ItemData.Freshness));
    }

    // The drying stage comes from the profile, never from dividing DryingMinutes here: the curve
    // runs past "fully dry" into Scorched, which a 0..100% clamp would hide.
    int32 StageIndex = INDEX_NONE;
    float StageProgressUnused = 0.f;
    if (UDataAssetDryingProfile::ResolveStageFor(ItemData, ItemDef, StageIndex, StageProgressUnused)
        && ItemDef->DryingProfile && ItemDef->DryingProfile->Stages.IsValidIndex(StageIndex))
    {
        const FGameplayTag& StageTag = ItemDef->DryingProfile->Stages[StageIndex].StageTag;
        if (StageTag.IsValid())
        {
            AddDetailRow(LOCTEXT("Drying", "Drying"), FText::FromString(TagLeaf(StageTag)));
        }
    }

    if (ItemData.bIsProcessed && !ItemData.ProcessingTags.IsEmpty())
    {
        TArray<FString> Names;
        for (const FGameplayTag& Tag : ItemData.ProcessingTags)
        {
            Names.Add(TagLeaf(Tag));
        }
        AddDetailRow(LOCTEXT("Processing", "Processing"), FText::FromString(FString::Join(Names, TEXT(", "))));
    }
}

#undef LOCTEXT_NAMESPACE
