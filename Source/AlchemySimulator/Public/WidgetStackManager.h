// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

	// Z-order for the first managed widget; each subsequent layer gets +1.
	static constexpr int32 BaseZOrder = 100;
};
