// Fill out your copyright notice in the Description page of Project Settings.


#include "AlchemyMinigameWidget.h"
#include "AlchemySimulatorPlayerController.h"
#include "Widgets/WidgetStackManager.h"

UAlchemyMinigameWidget::UAlchemyMinigameWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    CancelKeys = { EKeys::Escape, EKeys::Gamepad_FaceButton_Right };
}

void UAlchemyMinigameWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
}

void UAlchemyMinigameWidget::FinishMinigame(FMinigameResult Result)
{
    OnMinigameFinished.Broadcast(Result.bSuccess);
    OnMinigameResultFinished.Broadcast(Result);
}

void UAlchemyMinigameWidget::CancelMinigame()
{
    if (bMinigameClosed) return;

    // Close through the stack like every other path, so the manager's
    // OnWidgetPopped handler is the one place that clears the pending action.
    if (AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetOwningPlayer()))
    {
        if (PC->WidgetManager)
        {
            PC->WidgetManager->CloseWidget(this);
        }
    }
}

void UAlchemyMinigameWidget::OnOpened_Implementation()
{
    Super::OnOpened_Implementation();
    bMinigameClosed = false;
}

void UAlchemyMinigameWidget::OnClosed_Implementation()
{
    // Set before RemoveFromParent runs, so a focus-lost re-grab during teardown
    // cannot steal focus back from whatever RefreshInputMode focuses next.
    bMinigameClosed = true;
    Super::OnClosed_Implementation();
}

FReply UAlchemyMinigameWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();

    // Let the console through so ~ still works while iterating in PIE.
    if (Key == EKeys::Tilde)
    {
        return FReply::Unhandled();
    }

    if (CancelKeys.Contains(Key))
    {
        CancelMinigame();
        return FReply::Handled();
    }

    // Modal: this screen owns the keyboard.
    return FReply::Handled();
}
