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

	
	const int32 ZOrder = BaseZOrder + WidgetStack.Num();
	WidgetStack.Add(Widget);
	Widget->AddToViewport(ZOrder);
	Widget->OnOpened();
	OnWidgetPushed.Broadcast(Widget);
}

void UWidgetStackManager::PopWidget()
{
	if (WidgetStack.IsEmpty()) return;

	UBaseGameWidget* Top = WidgetStack.Last();
	if (!Top->CanClose()) return;

	WidgetStack.Pop();
	Top->OnClosed();
	Top->RemoveFromParent();
	OnWidgetPopped.Broadcast(Top);
}

void UWidgetStackManager::CloseAll()
{
	while (!WidgetStack.IsEmpty())
	{
		UBaseGameWidget* Top = WidgetStack.Pop();
		Top->OnClosed();
		Top->RemoveFromParent();
		OnWidgetPopped.Broadcast(Top);
	}
}

UBaseGameWidget* UWidgetStackManager::GetTopWidget() const
{
	return WidgetStack.IsEmpty() ? nullptr : WidgetStack.Last();
}
