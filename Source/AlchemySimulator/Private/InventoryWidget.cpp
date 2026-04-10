// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryWidget.h"
#include "InventoryComponent.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/Image.h"
#include "InventorySlotWidget.h"
#include "Components/TextBlock.h"

void UInventoryWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (!Inventory)
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            if (APawn* P = PC->GetPawn())
            {
                if (UInventoryComponent* Inv = P->FindComponentByClass<UInventoryComponent>())
                {
                    SetInventory(Inv);
                }
            }
        }
    }
    else
    {
        SetInventory(Inventory);
    }
}

void UInventoryWidget::NativeDestruct()
{
    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
    }

    Super::NativeDestruct();
}

void UInventoryWidget::HandleInventoryChanged()
{
    RebuildGrid();
}

void UInventoryWidget::SetInventory(UInventoryComponent* InInv)
{
    UE_LOG(LogTemp, Error, TEXT("Setting Inv"));

    if (Inventory)
    {
        Inventory->OnInventoryChanged.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
    }

    Inventory = InInv;

    if (Inventory)
    {
        Inventory->OnInventoryChanged.AddDynamic(this, &UInventoryWidget::HandleInventoryChanged);
        CapacitySlots = Inventory->MaxSlots;
    }

    RebuildGrid();
}

void UInventoryWidget::RebuildGrid()
{
    if (bIsRebuilding) return;
    TGuardValue<bool> Guard(bIsRebuilding, true);

    if (!Grid_Inventory || !EntryWidgetClass) return;

    if (!Inventory)
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            if (APawn* P = PC->GetPawn())
            {
                Inventory = P->FindComponentByClass<UInventoryComponent>();
            }
        }
    }

    if (!Inventory) return;


    Grid_Inventory->ClearChildren();
    Grid_Inventory->SetSlotPadding(FMargin(3.f));

    
    const int32 Cols = FMath::Max(1, NumColumns);

    const TArray<FInventorySlot>& Slots = Inventory->Slots;


    const int32 Count = FMath::Max(CapacitySlots, Slots.Num());

    for (int32 Index = 0; Index < Count; ++Index)
    {

        UInventorySlotWidget* Entry = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), EntryWidgetClass);
        if (!Entry) continue;

        Entry->OwnerInventory = Inventory;
        Entry->SlotIndex = Index;


        if (Index < Slots.Num())
        {
            Entry->Setup(Slots[Index]);
        }
        else
        {
            Entry->SetupEmpty();
        }

        if (UPanelSlot* PanelSlot = Grid_Inventory->AddChild(Entry))
        {
            if (UUniformGridSlot* GridSlot = Cast<UUniformGridSlot>(PanelSlot))
            {
                const int32 Row = Index / Cols;
                const int32 Col = Index % Cols;
                GridSlot->SetRow(Row);
                GridSlot->SetColumn(Col);
            }
        }
    }
}