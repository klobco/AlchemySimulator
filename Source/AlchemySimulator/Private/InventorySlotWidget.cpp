// Fill out your copyright notice in the Description page of Project Settings.


#include "InventorySlotWidget.h"
#include "ItemDefinitionBase.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "InventoryComponent.h"
#include "Components/Border.h"
#include "InvDragOperation.h"
#include "InventoryWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/TextBlock.h"



void UInventorySlotWidget::Setup(FInventorySlot slot)
{
	InvSlot = slot;
	Rebuild();
}

void UInventorySlotWidget::SetupEmpty()
{
    
    InvSlot = FInventorySlot{};

    if (SlotImage)
    {
        SlotImage->SetBrushFromTexture(nullptr);
        SlotImage->SetOpacity(0.f);
    }

    
    //if (SlotBorder)
    //{
    //    SlotBorder->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    //    FLinearColor GridTint(1.f, 1.f, 1.f, 0.12f);        
    //    SlotBorder->SetBrushColor(GridTint);
    //}
}

void UInventorySlotWidget::NativeConstruct()
{

	Super::NativeConstruct();
}



void UInventorySlotWidget::NativeDestruct()
{

	Super::NativeDestruct();

}

void UInventorySlotWidget::Rebuild() {

	if (SlotQuantity)
	{
		SlotQuantity->SetText(FText::AsNumber(InvSlot.Quantity));
	}

	if (SlotImage)
	{
		if (InvSlot.Item)
		{
			if (UTexture2D* Icon = InvSlot.Item->Icon)
			{
				SlotImage->SetBrushFromTexture(Icon, true);
				SlotImage->SetOpacity(1.f);
			}
			else
			{
				SlotImage->SetBrushFromTexture(nullptr);
			}
		}
		else
		{
			SlotImage->SetBrushFromTexture(nullptr);
			SlotImage->SetOpacity(0.f);
		}
	}

}

void UInventorySlotWidget::HandleClick() {

}


FReply UInventorySlotWidget::NativeOnMouseButtonDown(const FGeometry& G, const FPointerEvent& E)
{
    //if (E.GetEffectingButton() == EKeys::RightMouseButton)
    //{
    //    UE_LOG(LogTemp, Error, TEXT("Right mouse"));
    //    if (E.IsShiftDown()) OnSlotAction.Broadcast(this, SlotIndex, EInvAction::TransferAllToOther);
    //    else                 OnSlotAction.Broadcast(this, SlotIndex, EInvAction::TransferToOther);
    //    return FReply::Handled();
    //}

    if (E.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (OwnerInventory && OwnerInventory->Slots.IsValidIndex(SlotIndex) &&
            OwnerInventory->Slots[SlotIndex].Quantity > 0)
        {
            return UWidgetBlueprintLibrary::DetectDragIfPressed(E, this, EKeys::LeftMouseButton).NativeReply;
        }
    }
    return Super::NativeOnMouseButtonDown(G, E);
}

FReply UInventorySlotWidget::NativeOnPreviewMouseButtonDown(const FGeometry& G, const FPointerEvent& E)
{

    if (E.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        
        if (!OwnerInventory || !OwnerInventory->Slots.IsValidIndex(SlotIndex) ||
            OwnerInventory->Slots[SlotIndex].Quantity <= 0)
        {
            return FReply::Unhandled();
        }

        
        return UWidgetBlueprintLibrary::DetectDragIfPressed(E, this, EKeys::LeftMouseButton).NativeReply;
    }
    return Super::NativeOnPreviewMouseButtonDown(G, E);
}

void UInventorySlotWidget::NativeOnDragDetected(const FGeometry& G, const FPointerEvent& E, UDragDropOperation*& OutOp)
{
    if (!OwnerInventory || !OwnerInventory->Slots.IsValidIndex(SlotIndex)) return;

    const FInventorySlot& S = OwnerInventory->Slots[SlotIndex];
    if (S.Quantity <= 0) return;

    
    int32 Qty = S.Quantity;
    if (E.IsControlDown())      Qty = 1;
    else if (E.IsShiftDown())   Qty = FMath::Max(1, S.Quantity / 2);

    
    UInvDragOperation* Op = NewObject<UInvDragOperation>(this);
    Op->SourceWidget = GetTypedOuter<UInventoryWidget>(); 
    Op->SourceInventory = OwnerInventory;
    Op->FromIndex = SlotIndex;
    Op->Quantity = Qty;
    Op->SlotSnapshot = S;
    Op->Pivot = EDragPivot::CenterCenter; 

    
    UTexture2D* Icon = S.Item->Icon;
    UImage* DragImg = NewObject<UImage>(this);
    FSlateBrush Brush;
    Brush.SetResourceObject(Icon);
    Brush.ImageSize = FVector2D(64.f, 64.f); 
    DragImg->SetBrush(Brush);
    DragImg->SetOpacity(0.7f);

    Op->DefaultDragVisual = DragImg;

    OutOp = Op;
}

bool UInventorySlotWidget::NativeOnDrop(const FGeometry& G, const FDragDropEvent& E, UDragDropOperation* Operation)
{
    UInvDragOperation* Op = Cast<UInvDragOperation>(Operation);
    if (!Op || !OwnerInventory) return false;

    UInventoryComponent* FromInv = Op->SourceInventory;
    int32 FromIdx = Op->FromIndex;
    int32 Qty = Op->Quantity;

    UInventoryComponent* ToInv = OwnerInventory;
    int32 ToIdx = SlotIndex;

    if (!FromInv || !ToInv) return false;

    if (FromInv == ToInv)
    {
        
        if (FromIdx != ToIdx)
        {
            
            return FromInv->MoveWithin(FromIdx, ToIdx, Qty);
        }
        return false;
    }
    else
    {
        return FromInv->TransferTo(ToInv, FromIdx,ToIdx, Qty);
    }
}

void UInventorySlotWidget::NativeOnDragEnter(const FGeometry&, const FDragDropEvent&, UDragDropOperation* Op)
{
    //if (SlotBorder) SlotBorder->SetBrushColor(FLinearColor(1, 1, 0, 0.35f)); 
}
void UInventorySlotWidget::NativeOnDragLeave(const FDragDropEvent&, UDragDropOperation*)
{
    //if (SlotBorder) SlotBorder->SetBrushColor(FLinearColor::White);
}
bool UInventorySlotWidget::NativeOnDragOver(const FGeometry&, const FDragDropEvent&, UDragDropOperation*)
{
    return true;
}