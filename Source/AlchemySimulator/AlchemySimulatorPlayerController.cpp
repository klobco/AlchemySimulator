// Copyright Epic Games, Inc. All Rights Reserved.


#include "AlchemySimulatorPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "AlchemySimulator.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "InteractionDetectorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TableWidget.h"
#include "InteractionCameraRig.h"
#include "BasicInteractableStationObject.h"
#include "GameFramework/Character.h"
#include "BasePlant.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCursorWidget.h"
#include "BaseTool.h"
#include "ItemDefinitionBase.h"

void AAlchemySimulatorPlayerController::BeginPlay()
{

	UE_LOG(LogAlchemySimulator, Error, TEXT("Using my custom controller"));
	Super::BeginPlay();

	if (GetPawn())
	{
		BindToDetector(GetPawn());
	}

	if (!InteractionRig)
	{
		InteractionRig = GetWorld()->SpawnActor<AInteractionCameraRig>();
		if (InteractionRig)
		{
			InteractionRig->SetActorHiddenInGame(true);
			InteractionRig->SetActorEnableCollision(false);
		}
	}

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogAlchemySimulator, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AAlchemySimulatorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

bool AAlchemySimulatorPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AAlchemySimulatorPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	BindToDetector(InPawn);
}

void AAlchemySimulatorPlayerController::BindToDetector(APawn* InPawn)
{
	if (!InPawn) return;

	
	Detector = InPawn->FindComponentByClass<UInteractionDetectorComponent>();
	if (Detector)
	{
		Detector->OnFocusedChanged.RemoveAll(this);
		Detector->OnFocusedChanged.AddDynamic(this, &AAlchemySimulatorPlayerController::OnFocusedChanged);
	}
}

void AAlchemySimulatorPlayerController::OnFocusedChanged(UObject* NewObj, UObject* OldObj)
{
	UE_LOG(LogTemp, Error, TEXT("Focus changed: New=%s | Old=%s"),
		*GetNameSafe(NewObj),
		*GetNameSafe(OldObj)
	);
	CurrentTarget = NewObj;
}

void AAlchemySimulatorPlayerController::DoInteract() {
	if (CurrentTarget != nullptr)
	{
		//TODO potential problem when interacting with the other types of objects
		if (!Interacting)
		{
			OldTarget = GetViewTarget();
			ABasicInteractableStationObject* station = Cast<ABasicInteractableStationObject>(CurrentTarget.GetObject());
			if (station)
			{
				InteractionRig->SetActorLocation(station->InteractionViewPoint->GetComponentLocation());
				InteractionRig->SetActorRotation(station->InteractionViewPoint->GetComponentRotation());
				SetViewTargetWithBlend(InteractionRig, 0.35f);
				InteractionRig->EnableTilt();
				Interacting = true;
				SetupStationController(station);
				//TODO Do I need?
				IInteractable::Execute_Interact(CurrentTarget.GetObject(), GetPawn());
			}
			else
			{
				//TODO dangerous, assuming that it is ok with every other object.
				IInteractable::Execute_Interact(CurrentTarget.GetObject(), GetPawn());
			}

		}
		else
		{
			CloseWidget();
			RemoveStationController();
			InteractionRig->DisableTilt();
			SetViewTargetWithBlend(OldTarget, 0.35f);
			Interacting = false;
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Interacting with null"));
	}
}

void AAlchemySimulatorPlayerController::SetupStationController(ABasicInteractableStationObject* station) {

	CurrentStation = station;
	ACharacter* C = Cast<ACharacter>(GetPawn());
	if (C)
	{
		bEnableMouseOverEvents = true;
		bEnableClickEvents = true;
		bShowMouseCursor = true;
		SetIgnoreLookInput(true);
		SetIgnoreMoveInput(true);
	}
}

void AAlchemySimulatorPlayerController::RemoveStationController() {

	ResetActiveTool();
	CurrentStation = nullptr;
	ACharacter* C = Cast<ACharacter>(GetPawn());
	if (C)
	{
		bEnableMouseOverEvents = false;
		bEnableClickEvents = false;
		bShowMouseCursor = false;
		SetIgnoreLookInput(false);
		SetIgnoreMoveInput(false);
	}
}

void AAlchemySimulatorPlayerController::OpenWidget(TSubclassOf<UUserWidget> widgetClass) {

	if (!bIsOpenWidget)
	{
		InteractionRig->DisableTilt();
		if (!CurrentScreen && widgetClass)
		{
			CurrentScreen = CreateWidget<UUserWidget>(this, widgetClass);
		}
		if (CurrentScreen && !CurrentScreen->IsInViewport()) {
			CurrentScreen->AddToViewport(10);
			UTableWidget* TableWidget = Cast<UTableWidget>(CurrentScreen);
			if (TableWidget)
			{
				ABasicInteractableStationObject* Tab = Cast<ABasicInteractableStationObject>(CurrentTarget.GetObject());
				if (Tab)
				{
					TableWidget->SetTable(Tab);
				}
			}
		}

		bIsOpenWidget = true;
	}
	else
	{
		InteractionRig->EnableTilt();
		if (CurrentScreen && CurrentScreen->IsInViewport()) {

			UTableWidget* TableWidget = Cast<UTableWidget>(CurrentScreen);
			if (TableWidget)
			{
				TableWidget->SetTable(nullptr);
			}
			CurrentScreen->RemoveFromParent();
			CurrentScreen = nullptr;
			bIsOpenWidget = false;
		}
	}
}



void AAlchemySimulatorPlayerController::CloseWidget() {
	if (CurrentScreen && CurrentScreen->IsInViewport() && bIsOpenWidget) {
		InteractionRig->DisableTilt();
		CurrentScreen->RemoveFromParent();
		CurrentScreen = nullptr;
		bIsOpenWidget = false;
	}
}

void AAlchemySimulatorPlayerController::SetActiveTool(ABaseTool* tool){
	if (Interacting && CurrentStation)
	{
		UE_LOG(LogTemp, Error, TEXT("Setting active tool"));
		CurrentStation->SetActiveTool(tool);
	}

	if (tool && tool->Item && tool->Item->WorkbenchCursor && CursorWidgetClass)
	{
		if (!CursorWidgetInstance)
		{
			CursorWidgetInstance = CreateWidget<UCustomCursorWidget>(this, CursorWidgetClass);
		}
		CursorWidgetInstance->SetCursorTexture(tool->Item->WorkbenchCursor);
		SetMouseCursorWidget(EMouseCursor::Custom, CursorWidgetInstance);
		CurrentMouseCursor = EMouseCursor::Custom;
	}
}

void AAlchemySimulatorPlayerController::ResetActiveTool() {

	SetMouseCursorWidget(EMouseCursor::Custom, nullptr);
	CurrentMouseCursor = EMouseCursor::Default;

	CurrentStation->SetActiveTool(nullptr);
}

void AAlchemySimulatorPlayerController::DoBack() {

	UE_LOG(LogTemp, Error, TEXT("Backing"));
	if (CurrentScreen && CurrentScreen->IsInViewport() && bIsOpenWidget) {
		CloseWidget();
		InteractionRig->EnableTilt();
		return;
	}
	if (Interacting)
	{
		RemoveStationController();
		InteractionRig->DisableTilt();
		SetViewTargetWithBlend(OldTarget, 0.35f);
		Interacting = false;
		return;
	}
	
}