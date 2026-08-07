// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Minigame/MinigameManagerComponent.h"
#include "AlchemySimulatorPlayerController.h"
#include "IToolActionTarget.h"
#include "ItemDefinitions/ToolItemDefinition.h"
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

bool UMinigameManagerComponent::TryStartToolAction(UToolItemDefinition* Tool, AActor* Target, UPrimitiveComponent* HitComponent)
{
    if (!Tool || !Target) return false;
    if (!Target->Implements<UToolActionTarget>()) return false;

    for (const FToolAction& Action : Tool->Actions)
    {
        if (!Action.MinigameWidgetClass) continue;
        if (!IToolActionTarget::Execute_CanAcceptToolAction(Target, Tool, Action, HitComponent)) continue;

        // StartMinigame may call StopMinigame (which clears PendingAction), so record the
        // pending action only after the widget is up.
        StartMinigame(Action.MinigameWidgetClass);
        if (!ActiveMinigameWidget) return false;

        PendingAction.Target = Target;
        PendingAction.Tool = Tool;
        PendingAction.HitComponent = HitComponent;
        PendingAction.Action = Action;
        return true;
    }

    return false;
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
        // Bind the result delegate, not the bool one, so Score/QualityMultiplier survive.
        ActiveMinigameWidget->OnMinigameResultFinished.AddDynamic(this, &UMinigameManagerComponent::HandleMinigameResult);
        ActiveMinigameWidget->AddToViewport();

        PC->RefreshInputMode();
    }
}

void UMinigameManagerComponent::HandleMinigameResult(FMinigameResult Result)
{
    // Copy and clear before resolving: the target may start another action from inside its own
    // callback, and that new pending action must not be wiped when we return.
    const FPendingToolAction Pending = PendingAction;
    PendingAction.Reset();

    StopMinigame();

    if (AActor* Target = Pending.Target.Get())
    {
        UToolItemDefinition* Tool = Pending.Tool.Get();
        if (Tool && Target->Implements<UToolActionTarget>())
        {
            IToolActionTarget::Execute_ApplyToolActionResult(Target, Tool, Pending.Action, Result, Pending.HitComponent.Get());
        }
    }

    OnMinigameFinished.Broadcast(Result.bSuccess);
    OnMinigameResultFinished.Broadcast(Result);
}

void UMinigameManagerComponent::StopMinigame()
{
	// A cancelled or replaced minigame must never resolve its action later.
	PendingAction.Reset();

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
