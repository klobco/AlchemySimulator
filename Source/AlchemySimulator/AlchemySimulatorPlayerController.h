// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "AlchemySimulatorPlayerController.generated.h"

class UInputMappingContext;
class UUserWidget;

/**
 *  Basic PlayerController class for a third person game
 *  Manages input mappings
 */
UCLASS(abstract)
class AAlchemySimulatorPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category ="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Input mapping context setup */
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void SetupStationController(ABasicInteractableStationObject* station);



	UFUNCTION()
	void RemoveStationController();

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	virtual void OnPossess(APawn* InPawn) override;

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
	class UUserWidget* CurrentScreen = nullptr;

	void OpenWidget(TSubclassOf<UUserWidget> widgetClass);

	void CloseWidget();

	bool bIsOpenWidget = false;

	UPROPERTY()
	AActor* OldTarget;

};
