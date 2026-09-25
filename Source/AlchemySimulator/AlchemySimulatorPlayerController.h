 // Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputActionValue.h"
#include "AlchemySimulatorPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;
class UBaseGameWidget;
class UWidgetStackManager;
class UPlayerModeBase;
class UExplorationPlayerMode;
class UStationPlayerMode;
class ABasicInteractableStationObject;



/**
 *  Basic PlayerController class for a third person game.
 *  Manages input mappings and the modal widget stack.
 */
UCLASS(abstract)
class AAlchemySimulatorPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AAlchemySimulatorPlayerController();

protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts excluded on mobile */
	UPROPERTY(EditAnywhere, Category = "Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category = "Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;

	/**
	 * Enter station mode — pushes a UStationPlayerMode, which owns the whole
	 * enter half (view target, camera rig, tilt, table physics, pawn mesh).
	 * Callers only name the station. No-op if already at a station.
	 */
	UFUNCTION()
	void SetupStationController(ABasicInteractableStationObject* station);

	/**
	 * Leave station mode — pops the UStationPlayerMode, whose ExitMode is the
	 * exact mirror of its EnterMode. No-op if not at a station.
	 */
	UFUNCTION()
	void RemoveStationController();

	bool ShouldUseTouchControls() const;

	/** Push the mode that sits at the bottom of the stack. Called once, from BeginPlay. */
	void InitializePlayerModeStack();

	/** Apply whatever the active mode wants beyond input mode — currently the interaction detector. */
	void ApplyActiveModeState();

	/**
	 * Install the mapping contexts the active mode asks for, minus the actions
	 * it blocks. This is the only place that adds or removes a mapping context
	 * after startup — a key being live is mode data, not a branch in a handler.
	 *
	 * Blocked actions are removed by duplicating the context and unmapping them,
	 * so modifiers and triggers (WASD swizzles, rotate negation) survive.
	 * Cheap to call repeatedly: it early-outs when nothing has changed.
	 */
	void ApplyModeMappingContexts();

	/** Contexts currently installed by ApplyModeMappingContexts, including derived copies. */
	UPROPERTY()
	TArray<TObjectPtr<UInputMappingContext>> AppliedMappingContexts;

	/** What the last ApplyModeMappingContexts ran for, so repeats are free. */
	UPROPERTY()
	TObjectPtr<UPlayerModeBase> LastMappingMode = nullptr;
	bool bLastMappingUIOpen = false;

	UPROPERTY()
	class UInteractionDetectorComponent* Detector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TScriptInterface<class IInteractable> CurrentTarget;

	UFUNCTION()
	void OnFocusedChanged(UObject* NewObj, UObject* OldObj);

	/** Bound to the widget stack's push/pop delegates so input mode and camera tilt follow the stack. */
	UFUNCTION()
	void HandleWidgetStackChanged(UBaseGameWidget* Widget);

	void BindToDetector(APawn* InPawn);

public:
	/** Clear the station's active tool and the custom cursor that goes with it. */
	void ResetActiveTool();

	void DoInteract();
	void DoBack();

	/**
	 * Push a fully configured widget onto the modal stack and display it.
	 * Create and call any setup methods on the widget before passing it here.
	 * Input mode and camera tilt follow from HandleWidgetStackChanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void PushWidget(UBaseGameWidget* Widget);

	/**
	 * Close the top-most widget. Respects UBaseGameWidget::CanClose.
	 * Input mode and camera tilt follow from HandleWidgetStackChanged.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void PopWidget();

	/** Open the character screen on the widget stack. The exploration mode's answer to Back. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void OpenCharacterScreen();

	UPROPERTY(EditAnywhere)
	class AInteractionCameraRig* InteractionRig;

	/**
	 * Station mode is stored in exactly one place — the UStationPlayerMode on
	 * the mode stack. There is no separate flag or station pointer to keep in
	 * step with it; ask these instead.
	 */
	UFUNCTION(BlueprintPure, Category = "Interaction")
	bool IsAtStation() const { return GetCurrentStation() != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Interaction")
	ABasicInteractableStationObject* GetCurrentStation() const;

	/**
	 * The player mode stack. The bottom is always exploration — "no mode" is not
	 * a representable state, so RefreshInputMode always has a mode to ask.
	 * Mutate only through PushPlayerMode/PopPlayerMode.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Mode")
	TArray<TObjectPtr<UPlayerModeBase>> ModeStack;

	/** The mode that currently owns input. Never null after BeginPlay. */
	UFUNCTION(BlueprintPure, Category = "Player Mode")
	UPlayerModeBase* GetActivePlayerMode() const;

	/** Find the top-most mode of a class, or null. */
	UPlayerModeBase* FindPlayerMode(TSubclassOf<UPlayerModeBase> ModeClass) const;

	/**
	 * Push an already-configured mode: it goes on the stack, EnterMode runs, then
	 * the input mode and detector are refreshed. Configure the mode before pushing.
	 */
	UFUNCTION(BlueprintCallable, Category = "Player Mode")
	void PushPlayerMode(UPlayerModeBase* Mode);

	/**
	 * Remove a mode from the stack and run its ExitMode. The mode is off the
	 * stack before ExitMode runs, so anything it triggers already sees the mode
	 * gone. Refuses to pop the bottom (exploration) mode.
	 */
	UFUNCTION(BlueprintCallable, Category = "Player Mode")
	void PopPlayerMode(UPlayerModeBase* Mode);

	/** Class used for the bottom of the stack. Swap in Blueprint to change walking-around behaviour. */
	UPROPERTY(EditAnywhere, Category = "Player Mode")
	TSubclassOf<UExplorationPlayerMode> ExplorationModeClass;

	/** Class pushed when entering a station. */
	UPROPERTY(EditAnywhere, Category = "Player Mode")
	TSubclassOf<UStationPlayerMode> StationModeClass;

	UFUNCTION(BlueprintCallable, Category = "Tools")
	void SetActiveTool(class ABaseTool* tool);

	UFUNCTION()
	void RestoreCustomCursor();

	/**
	 * The single source of truth for input mode. Derives the correct mode from
	 * current state rather than having callers push one, in priority order:
	 * top stack widget > active player mode.
	 * Call this after any state change; never call SetInputMode directly.
	 *
	 * The top stack widget picks its own mode via UBaseGameWidget::IsModal():
	 * a modal widget gets FInputModeUIOnly and no Enhanced Input action reaches
	 * the game at all, so it must own its exit key; everything else gets
	 * FInputModeGameAndUI and the world stays clickable behind it. Dialogue and
	 * minigames are both modal stack widgets — neither is a special case here.
	 *
	 * With no widget open it installs whatever the top player mode asks for via
	 * UPlayerModeBase::GetInputSpec.
	 */
	void RefreshInputMode();

	/** Install one mode's spec. Shared by every branch of RefreshInputMode. */
	void ApplyInputSpec(const struct FPlayerModeInputSpec& Spec);

	UPROPERTY(EditAnywhere, Category = "Cursor")
	TSubclassOf<class UCustomCursorWidget> CursorWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Cursor")
	TSubclassOf<class UCharacterScreenWidget> CharacterScreenWidgetClass;

	UPROPERTY()
	TObjectPtr<class UCustomCursorWidget> CursorWidgetInstance = nullptr;

	UFUNCTION()
	bool TryHandleWorldDropFromScreenPosition(UInvDragOperation* DragOp, const FVector2D& ScreenPos);

	bool TraceFromScreenPosition(const FVector2D& ScreenPos, FHitResult& OutHit) const;


	/** The modal widget stack. Use PushWidget/PopWidget rather than accessing this directly. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetStackManager> WidgetManager;

	UPROPERTY()
	class UMinigameManagerComponent* MinigameManager = nullptr;

	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION()
	void StartWorldDrag(AActor* ActorToDrag);

	UFUNCTION()
	void StopWorldDrag();

	UFUNCTION()
	void RotateDraggedItem(const FInputActionValue& Value);

	UFUNCTION()
	void StopLeftMouseAction();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TObjectPtr<class UDialogueRuntimeComponent> DialogueRuntime;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UDialogueWidget> DialogueWidgetClass;

private:
	void DebugClick();

	UPROPERTY()
	AActor* DraggedActor = nullptr;

	bool bIsDraggingWorldActor = false;

	// Výška/rovina, po ktorej budeme ťahať objekt
	FVector DragPlaneOrigin = FVector::ZeroVector;
	FVector DragPlaneNormal = FVector::UpVector;

	// Offset, aby objekt neskočil presne stredom pod myš
	FVector DragOffset = FVector::ZeroVector;

	float RotationSpeed = 180.0f;

	UPROPERTY(EditAnywhere, Category = "Drag")
	float DragZLift = 10.0f;

	// Components that had physics disabled for the drag; restored on StopWorldDrag
	UPROPERTY()
	TArray<class UPrimitiveComponent*> DraggedPhysicsComponents;

	// Smoothed position written each tick so motion feels stable
	FVector DragSmoothedLocation = FVector::ZeroVector;

protected:

	UFUNCTION()
	void HandleDialogueStarted(class ANPCCharacter* NPC);
};
