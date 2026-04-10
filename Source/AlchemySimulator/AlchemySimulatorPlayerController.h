// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
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

	UPROPERTY()
	TScriptInterface<class IInteractable> CurrentTarget;

	UFUNCTION()
	void OnFocusedChanged(UObject* NewObj, UObject* OldObj);

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

	class AInteractionCameraRig* InteractionRig;

	UPROPERTY()
	bool Interacting = false;

	UFUNCTION(BlueprintCallable, Category = "Tools")
	void SetActiveTool(class ABaseTool* tool);

	UPROPERTY(EditAnywhere, Category = "Cursor")
	TSubclassOf<class UCustomCursorWidget> CursorWidgetClass;

	UPROPERTY()
	TObjectPtr<class UCustomCursorWidget> CursorWidgetInstance = nullptr;

	UPROPERTY()
	ABasicInteractableStationObject* CurrentStation = nullptr;

	UPROPERTY()
	AActor* OldTarget;

	/** The modal widget stack. Use PushWidget/PopWidget rather than accessing this directly. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<UWidgetStackManager> WidgetManager;
};
