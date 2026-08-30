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

	UFUNCTION()
	void SetupStationController(ABasicInteractableStationObject* station);

	UFUNCTION()
	void RemoveStationController();

	bool ShouldUseTouchControls() const;

	UPROPERTY()
	class UInteractionDetectorComponent* Detector = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	TScriptInterface<class IInteractable> CurrentTarget;

	UFUNCTION()
	void OnFocusedChanged(UObject* NewObj, UObject* OldObj);

	/** Bound to the widget stack's push/pop delegates so input mode follows the stack. */
	UFUNCTION()
	void HandleWidgetStackChanged(UBaseGameWidget* Widget);

	void BindToDetector(APawn* InPawn);
	void ResetActiveTool();

public:
	void DoInteract();
	void DoBack();

	/**
	 * Push a fully configured widget onto the modal stack and display it.
	 * Create and call any setup methods on the widget before passing it here.
	 * Also manages camera rig tilt (disables while any widget is open).
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void PushWidget(UBaseGameWidget* Widget);

	/**
	 * Close the top-most widget. Respects UBaseGameWidget::CanClose.
	 * Re-enables camera tilt when the stack becomes empty.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void PopWidget();

	UPROPERTY(EditAnywhere)
	class AInteractionCameraRig* InteractionRig;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interaction")
	bool Interacting = false;

	UFUNCTION(BlueprintCallable, Category = "Tools")
	void SetActiveTool(class ABaseTool* tool);

	UFUNCTION()
	void RestoreCustomCursor();

	/**
	 * The single source of truth for input mode. Derives the correct mode from
	 * current state rather than having callers push one, in priority order:
	 * active minigame > top stack widget > at a station > plain gameplay.
	 * Call this after any state change; never call SetInputMode directly.
	 */
	void RefreshInputMode();

	UPROPERTY(EditAnywhere, Category = "Cursor")
	TSubclassOf<class UCustomCursorWidget> CursorWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Cursor")
	TSubclassOf<class UCharacterScreenWidget> CharacterScreenWidgetClass;

	UPROPERTY()
	TObjectPtr<class UCustomCursorWidget> CursorWidgetInstance = nullptr;

	UFUNCTION()
	bool TryHandleWorldDropFromScreenPosition(UInvDragOperation* DragOp, const FVector2D& ScreenPos);

	bool TraceFromScreenPosition(const FVector2D& ScreenPos, FHitResult& OutHit) const;

	UPROPERTY()
	ABasicInteractableStationObject* CurrentStation = nullptr;

	UPROPERTY()
	AActor* OldTarget;

	/** The modal widget stack. Use PushWidget/PopWidget rather than accessing this directly. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetStackManager> WidgetManager;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Dialogue")
	class UDialogueRuntimeComponent* DialogueComponent = nullptr;

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
};
