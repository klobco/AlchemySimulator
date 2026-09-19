// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "DialogueOptionWidget.h"

void UDialogueOptionWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UDialogueOptionWidget::NativeDestruct()
{
    Super::NativeDestruct();
    DialogueOptionButton->OnClicked.RemoveDynamic(this, &UDialogueOptionWidget::OnDialogueOptionClicked);
}

void UDialogueOptionWidget::Setup(int32 InIndex, const FDialogueOption& InOption)
{
    Index = InIndex;
    CurrentOption = InOption;
    if (DialogueOptionButton)
    {
        DialogueOptionButton->OnClicked.AddDynamic(this, &UDialogueOptionWidget::OnDialogueOptionClicked);
    }
    if (DialogueOptionText)
    {
        DialogueOptionText->SetText(CurrentOption.Text);
    }
}


void UDialogueOptionWidget::OnDialogueOptionClicked()
{
   DialogueOptionClicked.Broadcast(Index);
}