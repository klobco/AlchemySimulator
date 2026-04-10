// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseGameWidget.generated.h"

/**
 * Base class for all widgets managed by UWidgetStackManager.
 * Inherit from this (instead of UUserWidget) for any widget that will be
 * pushed onto the modal widget stack.
 *
 * Lifecycle:
 *   1. Caller creates the widget:  CreateWidget<UMyWidget>(PC, WidgetClass)
 *   2. Caller configures it:       Widget->Setup(...)
 *   3. Caller pushes it:           PC->PushWidget(Widget)
 *      -> AddToViewport -> NativeConstruct -> OnOpened
 *   4. When closed by PopWidget:   OnClosed -> RemoveFromParent
 */
UCLASS(Abstract, BlueprintType, Blueprintable)
class ALCHEMYSIMULATOR_API UBaseGameWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Called after this widget is added to the viewport by the stack manager. */
	UFUNCTION(BlueprintNativeEvent, Category = "Widget Stack")
	void OnOpened();
	virtual void OnOpened_Implementation() {}

	/** Called just before this widget is removed from the viewport by the stack manager. */
	UFUNCTION(BlueprintNativeEvent, Category = "Widget Stack")
	void OnClosed();
	virtual void OnClosed_Implementation() {}

	/**
	 * Return false to block PopWidget from closing this widget.
	 * Useful for confirmation dialogs or widgets mid-animation.
	 * CloseAll() bypasses this check.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Widget Stack")
	bool CanClose() const;
	virtual bool CanClose_Implementation() const { return true; }
};
