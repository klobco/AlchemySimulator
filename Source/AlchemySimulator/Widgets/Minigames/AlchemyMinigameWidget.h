// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Menu/BaseGameWidget.h"
#include "DataStructHelpers.h"
#include "AlchemyMinigameWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinigameFinished, bool, bSuccess);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinigameResultFinished, FMinigameResult, Result);

/**
 * Base class for every minigame.
 *
 * A minigame is an ordinary modal widget on the widget stack — pushed and closed
 * by UMinigameManagerComponent, never added to the viewport directly. Being modal
 * is what gives it the keyboard (RefreshInputMode installs FInputModeUIOnly for a
 * modal top widget) and what makes the widgets beneath it unclickable.
 *
 * A subclass only has to play its game and call FinishMinigame once with the
 * result. It must NOT RemoveFromParent itself: the manager owns its lifetime, and
 * a widget that leaves the viewport behind the stack's back leaves a dead entry on it.
 */
UCLASS()
class ALCHEMYSIMULATOR_API UAlchemyMinigameWidget : public UBaseGameWidget
{
	GENERATED_BODY()

public:
    UAlchemyMinigameWidget(const FObjectInitializer& ObjectInitializer);

    UPROPERTY(BlueprintAssignable)
    FOnMinigameFinished OnMinigameFinished;

    UPROPERTY(BlueprintAssignable)
    FOnMinigameResultFinished OnMinigameResultFinished;

    /** Report the result. Call exactly once; the manager closes the widget and applies the action. */
    UFUNCTION(BlueprintCallable)
    void FinishMinigame(FMinigameResult Result);

    /**
     * Abandon the minigame with no result: the pending tool action is dropped and
     * the target is left untouched. Bound to CancelKeys by default.
     */
    UFUNCTION(BlueprintCallable)
    void CancelMinigame();

    /** True once the stack has closed this widget. Guard focus re-grabs and late timers on it. */
    UFUNCTION(BlueprintPure)
    bool IsMinigameClosed() const { return bMinigameClosed; }

    /**
     * Keys that abandon the minigame. Under FInputModeUIOnly the Back action never
     * reaches the game, so a modal screen has to own its own way out. Empty the
     * array on a Blueprint subclass to make a minigame that must be finished.
     */
    UPROPERTY(EditAnywhere, Category = "Minigame")
    TArray<FKey> CancelKeys;

    virtual bool IsModal_Implementation() const override { return true; }
    virtual void OnOpened_Implementation() override;
    virtual void OnClosed_Implementation() override;

protected:
    /** Makes the widget focusable, so RefreshInputMode's UIOnly focus actually lands and CancelKeys arrive. */
    virtual void NativeConstruct() override;

    /**
     * Handles CancelKeys and swallows every other key. Subclasses handle their own
     * keys first and call Super for the rest. An unhandled key would bubble into
     * Slate navigation and move focus off the minigame.
     */
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    bool bMinigameClosed = false;
};
