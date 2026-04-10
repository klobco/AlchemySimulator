// Fill out your copyright notice in the Description page of Project Settings.

#include "InventoryComponent.h"
#include "ItemMetadata.h"
#include "ItemDefinitionBase.h"
#include "ActionTabItemWidget.h"
#include "BasicInteractableStationObject.h"
#include "TableInventoryWidget.h"
#include "BasicWorkbench.h"
#include "BaseTool.h"
#include "Components/VerticalBox.h"
#include "TableWidget.h"

static FText GetActionText(EAction Action)
{
    const UEnum* Enum = StaticEnum<EAction>();
    return Enum ? Enum->GetDisplayNameTextByValue((int64)Action)
        : FText::FromString(TEXT("Invalid"));
}

void UTableWidget::NativeConstruct()
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
                    Inventory = Inv;
                    Inventory->OnInventoryChanged.AddDynamic(this, &UTableWidget::Rebuild);
                }
            }
        }
    }

    Rebuild();
}

void UTableWidget::NativeDestruct()
{
    Super::NativeDestruct();

}

void UTableWidget::Setup(UInventoryComponent* InInventory) {
    Inventory = InInventory;

    Rebuild();
}

void UTableWidget::Rebuild() {

    ActionItems->ClearChildren();
    InventoryItems->ClearChildren();

    

    //TODO REFACTOR DONT ITERATE ENUM IDIOTEN!
    for (EAction Action : TEnumRange<EAction>())
    {

        if (Action == EAction::None) continue;

        UActionTabWidget* Entry = CreateWidget<UActionTabWidget>(GetOwningPlayer(), EntryActionWidgetClass);
        if (!Entry) continue;


        Entry->Setup(GetActionText(Action), Action);

        Entry->ActionClicked.AddDynamic(this,&UTableWidget::SetAction);

        ActionItems->AddChild(Entry);
    }

    if (SelectedAction == EAction::AddToTable)
    {

        for (FInventorySlot InvSlot : Inventory->GetSlots()) {
            if (InvSlot.IsEmpty()) continue;

            if (InvSlot.Item->Category == EItemCategory::Tool || InvSlot.Item->Category == EItemCategory::Knife)
            {
                UActionTabItemWidget* Entry = CreateWidget<UActionTabItemWidget>(GetOwningPlayer(), EntryItemWidgetClass);
                if (!Entry) continue;


                Entry->Setup(InvSlot);

                Entry->ItemClicked.AddDynamic(this, &UTableWidget::ItemAction);

                InventoryItems->AddChild(Entry);
            }
        }
    }

    if (SelectedAction == EAction::RemoveFromTable && Table)
    {

        for (auto& tool : Table->Tools) {

            UActionTabItemWidget* Entry = CreateWidget<UActionTabItemWidget>(GetOwningPlayer(), EntryItemWidgetClass);
            if (!Entry) continue;


            Entry->Setup(Table->toolsInventory->Slots[tool.Key]);

            Entry->ItemClicked.AddDynamic(this, &UTableWidget::ItemAction);

            InventoryItems->AddChild(Entry);
        }
    }

}

void UTableWidget::SetAction(EAction Act) {
    SelectedAction = Act;
    Rebuild();
}

void UTableWidget::SetTable(ABasicInteractableStationObject* InTable) {
    Table = InTable;
    /*UE_LOG(LogTemp, Error, TEXT("SetTable: %s"), *InTable->GetName());*/
    Rebuild();
}

void UTableWidget::ItemAction(FInventorySlot InSlot) {

    if (SelectedAction == EAction::AddHerb)
    {

        if (!Table) return;

        if (Table->HerbsLimit < Table->herbsInventory->Slots.Num()) return;

        //if (!InSlot.Item || !InSlot.Item->WorldItem) return;

        if (!Inventory->RemoveSlotItem(InSlot)) return;

        UE_LOG(LogTemp, Error, TEXT("Herb Action Add"));

        if (ABasicWorkbench* workbench =  Cast<ABasicWorkbench>(Table))
        {
            workbench->AddHerb(InSlot);
        }
    }


    if (SelectedAction == EAction::AddToTable)
    {
        if (!Table) return;

        if (Table->ToolsLimit < Table->toolsInventory->Slots.Num()) return;

        if (!InSlot.Item || !InSlot.Item->WorldItem) return;

        if (!Inventory->RemoveSlotItem(InSlot)) return; 


        UE_LOG(LogTemp, Error, TEXT("Item Action Add"));



        UWorld* World = GetWorld();
        if (!World) return;

        FActorSpawnParameters Params;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        FRotator SpawnRotation(0.f, 0.f, 0.f);

        ABaseTool* SpawnedTool = World->SpawnActor<ABaseTool>(
            InSlot.Item->WorldItem,
            Table->GetActorLocation(),
            SpawnRotation,
            Params
        );

        if (SpawnedTool)
        {
            SpawnedTool->ToolMesh->SetSimulatePhysics(false);
            SpawnedTool->ToolMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            SpawnedTool->Tags.Empty();

            
            Table->AddTool(SpawnedTool);
        }
    }
    else if (SelectedAction == EAction::RemoveFromTable)
    {
        UE_LOG(LogTemp, Error, TEXT("Item Action Remove"));

        if (!Table) return;

        // Find the ABaseTool* in the table matching this inventory slot
        ABaseTool* ToolToRemove = nullptr;
        for (auto& Pair : Table->Tools)
        {
            const FInventorySlot& TableSlot = Table->toolsInventory->Slots[Pair.Key];
            if (TableSlot.Item == InSlot.Item && TableSlot.Instance.Quality == InSlot.Instance.Quality)
            {
                ToolToRemove = Pair.Value;
                break;
            }
        }

        if (!ToolToRemove) return;

        Table->RemoveTool(ToolToRemove);

        Inventory->AddItem(InSlot.Item, 1, InSlot.Instance);
    }



}