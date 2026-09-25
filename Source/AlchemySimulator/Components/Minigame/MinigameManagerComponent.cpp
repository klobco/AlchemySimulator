// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Minigame/MinigameManagerComponent.h"
#include "AlchemySimulatorPlayerController.h"
#include "IToolActionTarget.h"
#include "ItemDefinitions/ToolItemDefinition.h"
#include "Widgets/WidgetStackManager.h"
#include "Widgets/Menu/BaseGameWidget.h"
#include "Widgets/CustomCursorWidget.h"
#include "Framework/Application/SlateApplication.h"

UMinigameManagerComponent::UMinigameManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMinigameManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Every path that ends a minigame goes through the stack — result, cancel key,
	// StopMinigame, and CloseAll when the station is left — so listening here is
	// what guarantees a closed minigame can never resolve its action later.
	if (AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetOwner()))
	{
		if (PC->WidgetManager)
		{
			PC->WidgetManager->OnWidgetPopped.AddUniqueDynamic(this, &UMinigameManagerComponent::HandleWidgetClosed);
		}
	}
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
    if (!PC || !PC->WidgetManager) return;

    // A modal widget owns the screen outright, and a minigame is modal itself, so
    // this also refuses a second minigame over the first. Stacking a minigame on a
    // dialogue would leave the conversation running blind underneath it. Callers
    // treat "no widget" as a refusal and fall back (e.g. to dragging).
    if (UBaseGameWidget* Top = PC->WidgetManager->GetTopWidget())
    {
        if (Top->IsModal())
        {
            UE_LOG(LogTemp, Warning, TEXT("[Minigame] refused to start under the modal widget %s"), *GetNameSafe(Top));
            return;
        }
    }

    // Only reachable if something non-modal was pushed above a running minigame.
    if (ActiveMinigameWidget)
    {
        StopMinigame();
    }

    UAlchemyMinigameWidget* Widget = CreateWidget<UAlchemyMinigameWidget>(PC, MinigameWidgetClass);
    if (!Widget) return;

    // Set before the push: pushing broadcasts OnWidgetPushed, and anything reacting
    // to it should already see this as the active minigame.
    ActiveMinigameWidget = Widget;

    // Bind the result delegate, not the bool one, so Score/QualityMultiplier survive.
    Widget->OnMinigameResultFinished.AddUniqueDynamic(this, &UMinigameManagerComponent::HandleMinigameResult);

    // No CloseAll: the minigame sits on top of whatever is open (the table widget
    // stays visible), and the stack makes everything beneath a modal widget
    // unclickable. Input mode follows via the stack's OnWidgetPushed.
    PC->PushWidget(Widget);
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

	if (!ActiveMinigameWidget)
	{
		return;
	}

	AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetOwner());
	if (PC && PC->WidgetManager)
	{
		// HandleWidgetClosed does the rest.
		PC->WidgetManager->CloseWidget(ActiveMinigameWidget.Get());
	}
	else
	{
		// No stack to close it through — only possible during teardown.
		ActiveMinigameWidget->RemoveFromParent();
		ActiveMinigameWidget = nullptr;
	}
}

void UMinigameManagerComponent::HandleWidgetClosed(UBaseGameWidget* Widget)
{
	if (!Widget || Widget != ActiveMinigameWidget.Get())
	{
		return;
	}

	// However it was closed, it resolves nothing from here on: a late FinishMinigame
	// from a timer on the dying widget reaches no one.
	ActiveMinigameWidget->OnMinigameResultFinished.RemoveDynamic(this, &UMinigameManagerComponent::HandleMinigameResult);
	ActiveMinigameWidget = nullptr;
	PendingAction.Reset();

	// Input mode is not ours to touch — the controller refreshes it from its own
	// OnWidgetPopped handler. The tool cursor does not survive the UIOnly
	// round-trip, so put it back on the next tick, once both handlers have run.
	AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetOwner());
	if (PC && PC->IsAtStation() && PC->CurrentMouseCursor == EMouseCursor::Custom && PC->CursorWidgetInstance)
	{
		GetWorld()->GetTimerManager().SetTimerForNextTick(PC, &AAlchemySimulatorPlayerController::RestoreCustomCursor);
	}
}
