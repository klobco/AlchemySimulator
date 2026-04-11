// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicInteractableStationObject.h"
#include "InventoryWidget.h"
#include "InventoryComponent.h"
#include "TableInventoryWidget.h"

void UTableInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UTableInventoryWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UTableInventoryWidget::Setup(ABasicInteractableStationObject* station) {
    if (!station) return;

    if (TableInventoryWidget)
    {
        TableInventoryWidget->SetInventory(station->herbsInventory);
    }

    if (PlayerInventoryWidget)
    {
        UInventoryComponent* PlayerInv = nullptr;
        if (APlayerController* PC = GetOwningPlayer())
        {
            if (APawn* P = PC->GetPawn())
            {
                PlayerInv = P->FindComponentByClass<UInventoryComponent>();
            }
        }
        PlayerInventoryWidget->SetInventory(PlayerInv);
    }
}