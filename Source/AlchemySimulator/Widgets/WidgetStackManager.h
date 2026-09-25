// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SlateWrapperTypes.h"
#include "WidgetStackManager.generated.h"

class UBaseGameWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWidgetStackChanged, UBaseGameWidget*, Widget);

/**
 * Manages a stack of UBaseGameWidget instances as modal windows.
 *
 * Attach to a PlayerController. Callers create and configure widgets themselves,
 * then push via PushWidget — the manager owns Z-ordering, lifecycle events
 * (OnOpened / OnClosed), and stack teardown.
 *
 * Usage pattern:
 *   UMyWidget* W = CreateWidget<UMyWidget>(PC, MyWidgetClass);
 *   W->Setup(...);              // configure before pushing
 *   PC->PushWidget(W);         // manager takes it from here
 *
 * A modal widget (UBaseGameWidget::IsModal) blocks the widgets beneath it: they
 * stay on screen but are made HitTestInvisible until it closes. FInputModeUIOnly
 * only keeps input out of the *game* — Slate still routes clicks to any widget
 * under the cursor — so without this a minigame over the table widget would let
 * stray clicks reach the table.
 *
 * Every removal path (PopWidget, CloseWidget, CloseAll) broadcasts OnWidgetPopped,
 * so an owner that needs to react to its widget closing binds that once instead
 * of guessing which path closed it.
 */
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class ALCHEMYSIMULATOR_API UWidgetStackManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UWidgetStackManager();

	/**
	 * Push a widget onto the stack and add it to the viewport.
	 * Create and configure the widget before calling this.
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Stack")
	void PushWidget(UBaseGameWidget* Widget);

	/**
	 * Close the top-most widget. Respects CanClose() — does nothing if the widget vetoes.
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Stack")
	void PopWidget();

	/**
	 * Close one specific widget wherever it sits in the stack, bypassing CanClose.
	 * For owners ending their own widget's lifetime (the minigame manager): PopWidget
	 * takes whatever is on top, which is not necessarily the widget you mean.
	 * Does nothing if the widget is not on the stack.
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Stack")
	void CloseWidget(UBaseGameWidget* Widget);

	/**
	 * Close every widget unconditionally (bypasses CanClose).
	 * Call when leaving interaction mode entirely.
	 */
	UFUNCTION(BlueprintCallable, Category = "Widget Stack")
	void CloseAll();

	UFUNCTION(BlueprintPure, Category = "Widget Stack")
	bool HasOpenWidgets() const { return WidgetStack.Num() > 0; }

	UFUNCTION(BlueprintPure, Category = "Widget Stack")
	UBaseGameWidget* GetTopWidget() const;

	/** Fired after a widget is successfully pushed. */
	UPROPERTY(BlueprintAssignable, Category = "Widget Stack")
	FOnWidgetStackChanged OnWidgetPushed;

	/** Fired after a widget is removed (popped or closed by CloseAll). */
	UPROPERTY(BlueprintAssignable, Category = "Widget Stack")
	FOnWidgetStackChanged OnWidgetPopped;

private:
	UPROPERTY()
	TArray<TObjectPtr<UBaseGameWidget>> WidgetStack;

	/**
	 * Original visibility of each widget currently blocked by a modal widget above
	 * it, restored when the modal closes or the widget itself is removed.
	 */
	UPROPERTY()
	TMap<TObjectPtr<UBaseGameWidget>, ESlateVisibility> BlockedVisibility;

	/** Remove the widget at Index: lifecycle, viewport, blocking, broadcast. Shared by every close path. */
	void CloseAt(int32 Index);

	/** Make everything beneath the top-most modal widget unclickable, and restore everything else. */
	void UpdateModalBlocking();

	// Z-order for the first managed widget; each subsequent push gets +1.
	static constexpr int32 BaseZOrder = 100;

	// Monotonic while the stack is non-empty, so a widget pushed after CloseWidget
	// removed one from the middle can never share a Z-order with the one on top.
	int32 NextZOrder = BaseZOrder;
};
