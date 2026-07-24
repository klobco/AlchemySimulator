// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/InventoryComponent.h"
#include "Widgets/Inventory/InventoryWidget.h"
#include "CharacterScreenWidget.h"



void UCharacterScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();

    Setup();
}

void UCharacterScreenWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UCharacterScreenWidget::Setup()
{
    if (PlayerInventoryWidget)
    {
        PlayerInventoryWidget->SetInventory(GetOwningPlayerPawn()->FindComponentByClass<UInventoryComponent>());
    }

    PlayerInventoryWidget->SetInventory(GetOwningPlayerPawn()->FindComponentByClass<UInventoryComponent>());
    PlayerInventoryWidget->RebuildGrid();
}

void UCharacterScreenWidget::OnClosed_Implementation()
{
    // Implementation for closing the character screen widget
}