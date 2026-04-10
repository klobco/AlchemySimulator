// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionTabItemWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "ItemDefinitionBase.h"
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

    ItemName->SetText(ItemSlot.Item->DisplayName);
    ItemIcon->SetBrushFromTexture(ItemSlot.Item->Icon, true);
}