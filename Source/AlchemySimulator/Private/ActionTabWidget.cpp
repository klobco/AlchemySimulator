// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "ActionTabWidget.h"


void UActionTabWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ActionButton)
    {
        ActionButton->OnClicked.Clear();
        ActionButton->OnClicked.AddDynamic(this, &UActionTabWidget::HandleActionClicked);
    }
}

void UActionTabWidget::NativeDestruct()
{
    Super::NativeDestruct();

}

void UActionTabWidget::HandleActionClicked() {
    if (Action != EAction::None)
    {
        ActionClicked.Broadcast(Action);
    }
}

void UActionTabWidget::Setup(FText Text, EAction Acc) {
    ActionText = Text;
    Action = Acc;
    Refresh();
}

void UActionTabWidget::Refresh() {
    if (ActionName)
    {
        ActionName->SetText(ActionText);
    }
}