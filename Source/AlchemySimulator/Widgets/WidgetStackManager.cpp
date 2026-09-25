// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/WidgetStackManager.h"
#include "Widgets/Menu/BaseGameWidget.h"

UWidgetStackManager::UWidgetStackManager()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UWidgetStackManager::PushWidget(UBaseGameWidget* Widget)
{
	if (!Widget) return;

	if (WidgetStack.Contains(Widget))
	{
		UE_LOG(LogTemp, Warning, TEXT("Widget %s is already on the stack"), *GetNameSafe(Widget));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Pushing widget: %s"), *GetNameSafe(Widget));
	if (WidgetStack.IsEmpty())
	{
		NextZOrder = BaseZOrder;
	}
	WidgetStack.Add(Widget);
	Widget->AddToViewport(NextZOrder++);
	Widget->OnOpened();
	UpdateModalBlocking();
	OnWidgetPushed.Broadcast(Widget);
}

void UWidgetStackManager::PopWidget()
{
	if (WidgetStack.IsEmpty()) return;

	UBaseGameWidget* Top = WidgetStack.Last();
	if (!Top->CanClose()) return;

	UE_LOG(LogTemp, Warning, TEXT("Popping widget: %s"), *GetNameSafe(Top));
	CloseAt(WidgetStack.Num() - 1);
}

void UWidgetStackManager::CloseWidget(UBaseGameWidget* Widget)
{
	const int32 Index = WidgetStack.IndexOfByKey(Widget);
	if (Index == INDEX_NONE) return;

	UE_LOG(LogTemp, Warning, TEXT("Closing widget: %s"), *GetNameSafe(Widget));
	CloseAt(Index);
}

void UWidgetStackManager::CloseAll()
{
	while (!WidgetStack.IsEmpty())
	{
		CloseAt(WidgetStack.Num() - 1);
	}
}

void UWidgetStackManager::CloseAt(int32 Index)
{
	UBaseGameWidget* Widget = WidgetStack[Index];

	// Off the stack before OnClosed, so anything OnClosed triggers (a push, a
	// refresh) already sees it gone.
	WidgetStack.RemoveAt(Index);

	if (const ESlateVisibility* Saved = BlockedVisibility.Find(Widget))
	{
		Widget->SetVisibility(*Saved);
		BlockedVisibility.Remove(Widget);
	}

	Widget->OnClosed();
	Widget->RemoveFromParent();
	UpdateModalBlocking();
	OnWidgetPopped.Broadcast(Widget);
}

void UWidgetStackManager::UpdateModalBlocking()
{
	int32 ModalIndex = INDEX_NONE;
	for (int32 i = WidgetStack.Num() - 1; i >= 0; --i)
	{
		if (WidgetStack[i] && WidgetStack[i]->IsModal())
		{
			ModalIndex = i;
			break;
		}
	}

	for (int32 i = 0; i < WidgetStack.Num(); ++i)
	{
		UBaseGameWidget* Widget = WidgetStack[i];
		if (!Widget) continue;

		const bool bBlocked = i < ModalIndex;
		const bool bIsBlocked = BlockedVisibility.Contains(Widget);

		if (bBlocked && !bIsBlocked)
		{
			// Still drawn, no longer clickable. Hidden/Collapsed widgets are left
			// alone, but recorded, so restoring is the same for every widget.
			const ESlateVisibility Current = Widget->GetVisibility();
			BlockedVisibility.Add(Widget, Current);
			if (Current == ESlateVisibility::Visible || Current == ESlateVisibility::SelfHitTestInvisible)
			{
				Widget->SetVisibility(ESlateVisibility::HitTestInvisible);
			}
		}
		else if (!bBlocked && bIsBlocked)
		{
			Widget->SetVisibility(BlockedVisibility.FindChecked(Widget));
			BlockedVisibility.Remove(Widget);
		}
	}
}

UBaseGameWidget* UWidgetStackManager::GetTopWidget() const
{
	return WidgetStack.IsEmpty() ? nullptr : WidgetStack.Last();
}
