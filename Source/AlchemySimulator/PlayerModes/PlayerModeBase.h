// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PlayerModeBase.generated.h"

class AAlchemySimulatorPlayerController;
class UUserWidget;
class UInputMappingContext;
class UInputAction;

/** Which FInputMode* the controller should install for a mode. */
UENUM(BlueprintType)
enum class EPlayerInputModeKind : uint8
{
	/** FInputModeGameOnly — cursor hidden, no UI routing. */
	GameOnly,
	/** FInputModeGameAndUI — the world stays clickable behind the UI. */
	GameAndUI,
	/** FInputModeUIOnly — fully modal, no Enhanced Input action reaches the game. */
	UIOnly
};

/**
 * What a mode wants from the input system. A mode declares this instead of
 * calling SetInputMode itself — AAlchemySimulatorPlayerController::RefreshInputMode
 * is still the only place that may install a mode.
 */
USTRUCT(BlueprintType)
struct FPlayerModeInputSpec
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Player Mode")
	EPlayerInputModeKind Kind = EPlayerInputModeKind::GameOnly;

	UPROPERTY(BlueprintReadWrite, Category = "Player Mode")
	bool bShowCursor = false;

	/** Gate AddMovementInput/AddControllerYawInput. Does NOT gate Jump/Interact/Back — only UIOnly does. */
	UPROPERTY(BlueprintReadWrite, Category = "Player Mode")
	bool bSuppressPawnInput = true;

	/**
	 * With no widget to focus, focus the viewport explicitly — otherwise the
	 * first world click is eaten by Slate's focus change.
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Player Mode")
	bool bFocusGameViewport = false;

	/** Optional widget to focus. Ignored for GameOnly. */
	UPROPERTY(BlueprintReadWrite, Category = "Player Mode")
	TWeakObjectPtr<UUserWidget> WidgetToFocus;
};

/**
 * One player mode: exploration, being at a station, a conversation, a minigame.
 *
 * A mode is an object that *declares* what it needs rather than a branch that
 * does it. It owns both halves of its own enter/exit, so the two cannot drift
 * apart, and it hands RefreshInputMode a spec instead of calling SetInputMode.
 *
 * Modes live on AAlchemySimulatorPlayerController's mode stack — push one with
 * PushPlayerMode, remove it with PopPlayerMode; both call EnterMode/ExitMode and
 * refresh the input mode for you. Never call EnterMode/ExitMode directly.
 *
 * Adding a mode means one new subclass (and, from Stage 3 on, one input mapping
 * context asset) — not a new branch in DoInteract, DoBack and RefreshInputMode.
 */
UCLASS(Abstract, Blueprintable, BlueprintType)
class ALCHEMYSIMULATOR_API UPlayerModeBase : public UObject
{
	GENERATED_BODY()

public:
	/** Apply this mode's side effects. Called by PushPlayerMode, after the mode is on the stack. */
	virtual void EnterMode();

	/** Undo exactly what EnterMode did. Called by PopPlayerMode, after the mode is off the stack. */
	virtual void ExitMode();

	/** What the input system should look like while this mode is on top. */
	virtual FPlayerModeInputSpec GetInputSpec() const;

	/**
	 * Whether UInteractionDetectorComponent should keep scanning. A mode that
	 * does not want it gets focus highlighting and re-targeting switched off
	 * for its whole lifetime.
	 */
	virtual bool WantsInteractionDetector() const { return true; }

	/** For logging only. */
	virtual FName GetModeName() const { return TEXT("None"); }

	/**
	 * Which mapping contexts are installed while this mode is active.
	 * Empty (the default) means "the controller's DefaultMappingContexts" — a
	 * mode only names contexts when it needs *different* bindings, not merely
	 * fewer. Removing keys is BlockedActions' job.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Player Mode|Input")
	TArray<TSoftObjectPtr<UInputMappingContext>> MappingContexts;

	/**
	 * Actions unmapped from the installed contexts for this mode's whole
	 * lifetime. This is the real gate: SetIgnoreMove/LookInput only stop
	 * AddMovementInput/AddControllerYawInput, so Jump, Interact and Back stay
	 * live under everything except FInputModeUIOnly.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Player Mode|Input")
	TArray<TSoftObjectPtr<UInputAction>> BlockedActions;

	/** Additionally unmapped while any widget or minigame is up. */
	UPROPERTY(EditDefaultsOnly, Category = "Player Mode|Input")
	TArray<TSoftObjectPtr<UInputAction>> BlockedActionsWhileUIOpen;

	UPROPERTY(EditDefaultsOnly, Category = "Player Mode|Input")
	int32 MappingPriority = 0;

	/** Resolve MappingContexts. Empty output means "use the controller defaults". */
	virtual void GetMappingContexts(TArray<const UInputMappingContext*>& OutContexts) const;

	/** Resolve BlockedActions, plus BlockedActionsWhileUIOpen when bUIOpen. */
	virtual void GetBlockedActions(bool bUIOpen, TArray<const UInputAction*>& OutActions) const;

	/**
	 * The mode's answer to the Back action, asked only when no widget is open.
	 * Return true if handled. This is what keeps "what Escape does" out of
	 * AAlchemySimulatorPlayerController::DoBack — a new mode brings its own
	 * answer instead of adding a branch there.
	 */
	virtual bool HandleBackAction() { return false; }

	UFUNCTION(BlueprintPure, Category = "Player Mode")
	AAlchemySimulatorPlayerController* GetPlayerController() const { return OwningController; }

	/** Set by PushPlayerMode before EnterMode runs. */
	UPROPERTY(BlueprintReadOnly, Category = "Player Mode")
	TObjectPtr<AAlchemySimulatorPlayerController> OwningController = nullptr;

	// UObject has no world of its own; borrow the controller's so timers and
	// gameplay statics work from inside a mode.
	virtual UWorld* GetWorld() const override;
};
