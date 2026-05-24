// Fill out your copyright notice in the Description page of Project Settings.


#include "MinigameManagerComponent.h"
#include "AlchemyMinigameWidget.h"
#include "AlchemySimulatorPlayerController.h"
#include "WidgetStackManager.h"
#include "CustomCursorWidget.h"
#include "Framework/Application/SlateApplication.h"

UMinigameManagerComponent::UMinigameManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMinigameManagerComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMinigameManagerComponent::StartMinigame(TSubclassOf<UAlchemyMinigameWidget> MinigameWidgetClass)
{
    if (!MinigameWidgetClass) return;

    AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetOwner());
    if (!PC) return;

    PC->WidgetManager->CloseAll();
    bWasInStation = PC->Interacting;

    if (ActiveMinigameWidget)
    {
        StopMinigame();
    }

    ActiveMinigameWidget = CreateWidget<UAlchemyMinigameWidget>(PC, MinigameWidgetClass);

    if (ActiveMinigameWidget)
    {
        ActiveMinigameWidget->OnMinigameFinished.AddDynamic(this, &UMinigameManagerComponent::HandleMinigameFinished);
        ActiveMinigameWidget->AddToViewport();

        FInputModeUIOnly InputMode;
        InputMode.SetWidgetToFocus(ActiveMinigameWidget->TakeWidget());
        PC->SetInputMode(InputMode);
        PC->bShowMouseCursor = true;
    }
}

void UMinigameManagerComponent::HandleMinigameFinished(bool bSuccess)
{
    StopMinigame();
    OnMinigameFinished.Broadcast(bSuccess);
}

void UMinigameManagerComponent::StopMinigame()
{
	if (ActiveMinigameWidget)
	{
		ActiveMinigameWidget->RemoveFromParent();
		ActiveMinigameWidget = nullptr;

		AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetOwner());
		if (PC)
		{
			if (bWasInStation)
			{
				FSlateApplication::Get().SetAllUserFocusToGameViewport();
				
				FInputModeGameAndUI StationInputMode;
				StationInputMode.SetHideCursorDuringCapture(false);
				StationInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				PC->SetInputMode(StationInputMode);
				PC->bShowMouseCursor = true;

				if (PC->CurrentMouseCursor == EMouseCursor::Custom && PC->CursorWidgetInstance)
				{
					GetWorld()->GetTimerManager().SetTimerForNextTick(PC, &AAlchemySimulatorPlayerController::RestoreCustomCursor);
				}

			}
			else
			{
				FInputModeGameOnly GameOnlyMode;
				GameOnlyMode.SetConsumeCaptureMouseDown(false);
				PC->SetInputMode(GameOnlyMode);
				PC->bShowMouseCursor = false;
			}
			bWasInStation = false;
		}
	}
}