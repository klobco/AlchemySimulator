// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Widgets/Minigames/AlchemyMinigameWidget.h"
#include "ItemMetadata.h"
#include "MinigameManagerComponent.generated.h"

class UToolItemDefinition;
class UPrimitiveComponent;

/**
 * The tool action waiting on the active minigame's result.
 *
 * Held as weak pointers so a target destroyed mid-minigame simply resolves to nothing, and
 * cleared by StopMinigame() so a cancelled minigame can never resolve later. This replaces the
 * old pattern where each target AddDynamic'd itself to the manager's delegate — those bindings
 * leaked whenever a minigame was cancelled instead of completed.
 */
USTRUCT()
struct FPendingToolAction
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> Target;

	UPROPERTY()
	TWeakObjectPtr<UToolItemDefinition> Tool;

	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> HitComponent;

	/** Stored by value, not by index — the tool's Actions array may change while the minigame runs. */
	UPROPERTY()
	FToolAction Action;

	void Reset()
	{
		Target.Reset();
		Tool.Reset();
		HitComponent.Reset();
		Action = FToolAction();
	}
};

/**
 * Runs tool actions: picks the action, shows its minigame, and applies the result.
 *
 * The minigame widget itself lives on the ordinary widget stack as a modal widget
 * — this component does not manage the viewport, Z-order or input mode. It only
 * owns *which* minigame is running and *what* action it resolves. Every way a
 * minigame can end (result, cancel key, StopMinigame, CloseAll on leaving the
 * station) closes it through the stack, and HandleWidgetClosed is the single place
 * that clears the state afterwards.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ALCHEMYSIMULATOR_API UMinigameManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMinigameManagerComponent();

protected:
	virtual void BeginPlay() override;

public:
    UPROPERTY(BlueprintAssignable)
    FOnMinigameFinished OnMinigameFinished;

    /** Full result, including Score and QualityMultiplier. Prefer this over OnMinigameFinished. */
    UPROPERTY(BlueprintAssignable)
    FOnMinigameResultFinished OnMinigameResultFinished;

    /**
     * Run the first action on Tool that Target accepts, then apply its result to Target.
     *
     * Returns false when the tool has nothing to do here (wrong tool, target refuses, no
     * minigame configured) so the caller can fall through to its default click behaviour.
     */
    UFUNCTION(BlueprintCallable, Category = "Minigame")
    bool TryStartToolAction(UToolItemDefinition* Tool, AActor* Target, UPrimitiveComponent* HitComponent);

    /**
     * Push a minigame onto the widget stack, above whatever is open. Refuses (leaving
     * GetActiveMinigameWidget null) under a modal widget — a dialogue, or another
     * minigame — so callers treat a null widget as "not started".
     */
    UFUNCTION(BlueprintCallable)
    void StartMinigame(TSubclassOf<UAlchemyMinigameWidget> MinigameWidgetClass);

    /** Abandon the active minigame without a result. Its pending action is dropped. */
	UFUNCTION(BlueprintCallable)
    void StopMinigame();

    /** The minigame currently on screen, or null. */
    UFUNCTION(BlueprintPure)
    UAlchemyMinigameWidget* GetActiveMinigameWidget() const { return ActiveMinigameWidget; }

private:
    UPROPERTY()
    TObjectPtr<UAlchemyMinigameWidget> ActiveMinigameWidget;

    UPROPERTY()
    FPendingToolAction PendingAction;

    UFUNCTION()
    void HandleMinigameResult(FMinigameResult Result);

    /** Bound to the stack's OnWidgetPopped: the one place minigame state is cleared, whoever closed it. */
    UFUNCTION()
    void HandleWidgetClosed(UBaseGameWidget* Widget);

};
