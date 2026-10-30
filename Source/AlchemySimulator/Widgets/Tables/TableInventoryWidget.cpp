// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Stations/BasicInteractableStationObject.h"
#include "Widgets/Inventory/InventoryWidget.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Widgets/Tables/TableInventoryWidget.h"

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
        // Each station type answers for itself, so a new one needs no edit here.
        UInventoryComponent* StationInv = station->GetStationInventory();

        // Worth saying out loud: UInventoryWidget falls back to the player's own inventory
        // when handed null, so a station that forgot to override GetStationInventory would
        // otherwise show the player's bag in the station panel and look almost right.
        if (!StationInv)
        {
            UE_LOG(LogTemp, Warning, TEXT("[Station] %s has no station inventory - does it override GetStationInventory()?"),
                *station->GetName());
        }

        TableInventoryWidget->SetInventory(StationInv);
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