// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Tables/ActionTabItemWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "ItemDefinitions/ItemPresentationLibrary.h"
#include "Components/Image.h"

void UActionTabItemWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ItemButton)
    {
        ItemButton->OnClicked.Clear();
        ItemButton->OnClicked.AddDynamic(this, &UActionTabItemWidget::HandleActionClicked);
    }
}

void UActionTabItemWidget::NativeDestruct()
{
    Super::NativeDestruct();

}

void UActionTabItemWidget::HandleActionClicked() {
    ItemClicked.Broadcast(ItemSlot);
}

void UActionTabItemWidget::Setup(const FInventorySlot& InStack) {
    ItemSlot = InStack;
    Refresh();
}

void UActionTabItemWidget::Refresh() {

    if (!ItemSlot.Item) return;

    ItemName->SetText(UItemPresentationLibrary::ResolveItemDisplayName(ItemSlot.Instance, ItemSlot.Item));
    ItemIcon->SetBrushFromTexture(UItemPresentationLibrary::ResolveItemIcon(ItemSlot.Instance, ItemSlot.Item), true);
}
