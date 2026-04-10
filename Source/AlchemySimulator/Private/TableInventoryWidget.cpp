// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicInteractableStationObject.h"
#include "InventoryWidget.h"
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
    TableInventoryWidget->SetInventory(station->herbsInventory);
}