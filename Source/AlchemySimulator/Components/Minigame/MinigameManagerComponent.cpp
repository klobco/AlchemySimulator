// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Minigame/MinigameManagerComponent.h"
#include "AlchemySimulatorPlayerController.h"
#include "Widgets/WidgetStackManager.h"
#include "Widgets/CustomCursorWidget.h"
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

    if (ActiveMinigameWidget)
    {
        StopMinigame();
    }

    ActiveMinigameWidget = CreateWidget<UAlchemyMinigameWidget>(PC, MinigameWidgetClass);

    if (ActiveMinigameWidget)
    {
        ActiveMinigameWidget->OnMinigameFinished.AddDynamic(this, &UMinigameManagerComponent::HandleMinigameFinished);
        ActiveMinigameWidget->AddToViewport();

        PC->RefreshInputMode();
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
			// ActiveMinigameWidget is already null, so this falls through to whatever
			// state we're returning to (station or plain gameplay).
			PC->RefreshInputMode();

			if (PC->Interacting && PC->CurrentMouseCursor == EMouseCursor::Custom && PC->CursorWidgetInstance)
			{
				GetWorld()->GetTimerManager().SetTimerForNextTick(PC, &AAlchemySimulatorPlayerController::RestoreCustomCursor);
			}
		}
	}
}