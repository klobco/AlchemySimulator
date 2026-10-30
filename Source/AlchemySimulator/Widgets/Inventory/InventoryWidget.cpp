// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/InventoryWidget.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "Components/Image.h"
#include "Widgets/Inventory/InventorySlotWidget.h"
#include "AlchemySimulatorPlayerController.h"
#include "InvDragOperation.h"
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
    if (IsValid(Inventory))
    {
        Inventory->OnInventoryChangedType.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
    }

    Super::NativeDestruct();
}

void UInventoryWidget::SetInventory(UInventoryComponent* InInv)
{

    if (IsValid(Inventory))
    {
        Inventory->OnInventoryChangedType.RemoveDynamic(this, &UInventoryWidget::HandleInventoryChanged);
    }

    Inventory = InInv;

    if (Inventory)
    {
        // AddUnique: AddDynamic does not de-duplicate, and SetInventory can be called more
        // than once on one widget.
        Inventory->OnInventoryChangedType.AddUniqueDynamic(this, &UInventoryWidget::HandleInventoryChanged);
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

    // Sized up front and written by index, so a slot whose widget failed to create leaves a null
    // rather than shifting every later entry out of alignment with its slot.
    SlotWidgets.Reset();
    SlotWidgets.SetNum(Count);

    for (int32 Index = 0; Index < Count; ++Index)
    {

        UInventorySlotWidget* Entry = CreateWidget<UInventorySlotWidget>(GetOwningPlayer(), EntryWidgetClass);
        if (!Entry) continue;

        SlotWidgets[Index] = Entry;
        Entry->OwnerInventory = Inventory;
        Entry->SlotIndex = Index;


        if (Index < Slots.Num() && !Slots[Index].IsEmpty())
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

void UInventoryWidget::RefreshSlot(int32 SlotIndex)
{
    if (!Inventory || !SlotWidgets.IsValidIndex(SlotIndex)) return;

    UInventorySlotWidget* Entry = SlotWidgets[SlotIndex];
    if (!Entry) return;

    if (Inventory->Slots.IsValidIndex(SlotIndex) && !Inventory->Slots[SlotIndex].IsEmpty())
    {
        Entry->Setup(Inventory->Slots[SlotIndex]);
    }
    else
    {
        Entry->SetupEmpty();
    }
}

void UInventoryWidget::HandleInventoryChanged(EInventoryChangeType ChangeType, int32 SlotIndex)
{
    // The delegate's contract: INDEX_NONE means one operation touched several slots and the
    // listener should treat it as a full refresh. Anything else is exactly one slot.
    if (SlotIndex == INDEX_NONE)
    {
        RebuildGrid();
        return;
    }

    RefreshSlot(SlotIndex);
}