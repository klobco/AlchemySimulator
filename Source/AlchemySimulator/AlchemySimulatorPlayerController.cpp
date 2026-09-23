// Copyright Epic Games, Inc. All Rights Reserved.

#include "AlchemySimulatorPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Blueprint/UserWidget.h"
#include "AlchemySimulator.h"
#include "Widgets/Input/SVirtualJoystick.h"
#include "Components/InteractionDetectorComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "InteractionCameraRig.h"
#include "Actors/Stations/BasicInteractableStationObject.h"
#include "GameFramework/Character.h"
#include "Actors/Plants/BasePlant.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AlchemySimulatorCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "InvDragOperation.h"
#include "Actors/Stations/BasicWorkbench.h"
#include "Widgets/CustomCursorWidget.h"
#include "Actors/Tools/BaseTool.h"
#include "Actors/Plants/PlantPart.h"
#include "Widgets/Dialogue/DialogueWidget.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "Components/Minigame/MinigameManagerComponent.h"
#include "Widgets/Menu/BaseGameWidget.h"
#include "Widgets/WidgetStackManager.h"
#include "DrawDebugHelpers.h"
#include "Characters/NPCCharacter.h"
#include "Widgets/Menu/CharacterScreenWidget.h"
#include "Components/PrimitiveComponent.h"
#include "Components/Dialogue/DialogueRuntimeComponent.h"
#include "Framework/Application/SlateApplication.h"

AAlchemySimulatorPlayerController::AAlchemySimulatorPlayerController()
{
	WidgetManager = CreateDefaultSubobject<UWidgetStackManager>(TEXT("WidgetManager"));
	DialogueRuntime = CreateDefaultSubobject<UDialogueRuntimeComponent>(TEXT("DialogueRuntime"));
	MinigameManager = CreateDefaultSubobject<UMinigameManagerComponent>(TEXT("MinigameManager"));
}

void AAlchemySimulatorPlayerController::BeginPlay()
{
	UE_LOG(LogAlchemySimulator, Error, TEXT("Using my custom controller"));
	Super::BeginPlay();

	if (GetPawn())
	{
		BindToDetector(GetPawn());
	}

	// Input mode follows the widget stack — CloseAll broadcasts per popped widget,
	// so every path that changes the stack refreshes the mode automatically.
	if (WidgetManager)
	{
		WidgetManager->OnWidgetPushed.AddDynamic(this, &AAlchemySimulatorPlayerController::HandleWidgetStackChanged);
		WidgetManager->OnWidgetPopped.AddDynamic(this, &AAlchemySimulatorPlayerController::HandleWidgetStackChanged);
	}


	if (!InteractionRig)
	{
		InteractionRig = GetWorld()->SpawnActor<AInteractionCameraRig>();
		if (InteractionRig)
		{
			InteractionRig->SetActorHiddenInGame(true);
			InteractionRig->SetActorEnableCollision(false);
			InteractionRig->CameraArm->bDoCollisionTest = false;
		}
	}

	// only spawn touch controls on local player controllers
	if (ShouldUseTouchControls() && IsLocalPlayerController())
	{
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);
		if (MobileControlsWidget)
		{
			MobileControlsWidget->AddToPlayerScreen(0);
		}
		else
		{
			UE_LOG(LogAlchemySimulator, Error, TEXT("Could not spawn mobile controls widget."));
		}
	}

	DialogueRuntime->OnDialogueStarted.AddDynamic(
    this, &AAlchemySimulatorPlayerController::HandleDialogueStarted);
}

void AAlchemySimulatorPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}

	// InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AAlchemySimulatorPlayerController::DebugClick);
}

bool AAlchemySimulatorPlayerController::ShouldUseTouchControls() const
{
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

void AAlchemySimulatorPlayerController::DoInteract()
{

	if(bIsDraggingWorldActor) return;

	// Leaving is decided by station state alone, never by what the detector is
	// currently focused on — the focus can go null while standing at a station,
	// and requiring a target here used to make E unable to exit.
	if (IsAtStation())
	{
		RemoveStationController();
		return;
	}

	if (CurrentTarget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Interacting with null"));
		return;
	}

	if (ABasicInteractableStationObject* station = Cast<ABasicInteractableStationObject>(CurrentTarget.GetObject()))
	{
		// Every side effect of entering (view target, tilt, physics, mesh, input
		// mode) belongs to SetupStationController so the exit stays a mirror.
		SetupStationController(station);
		IInteractable::Execute_Interact(CurrentTarget.GetObject(), GetPawn());
	}
	else if (ANPCCharacter* NPC = Cast<ANPCCharacter>(CurrentTarget.GetObject()))
	{
		// Starting only. UDialogueWidget is modal, so once it is up this
		// action cannot fire again - ending the conversation is the widget's
		// own exit key. StartDialogue self-guards on IsInDialogue().
		DialogueRuntime->StartDialogue(NPC, NPC->DialogueProviderClass);
	}
	else
	{
		IInteractable::Execute_Interact(CurrentTarget.GetObject(), GetPawn());
	}
}

void AAlchemySimulatorPlayerController::SetupStationController(ABasicInteractableStationObject* station)
{
	if (!station || IsAtStation())
	{
		return;
	}

	CurrentStation = station;

	// Camera: frame the station and remember where to blend back to.
	PreStationViewTarget = GetViewTarget();
	if (InteractionRig && station->InteractionViewPoint)
	{
		InteractionRig->SetActorLocation(station->InteractionViewPoint->GetComponentLocation());
		InteractionRig->SetActorRotation(station->InteractionViewPoint->GetComponentRotation());
		SetViewTargetWithBlend(InteractionRig, 0.35f);
		InteractionRig->EnableTilt();
	}

	if (ABasicWorkbench* bench = Cast<ABasicWorkbench>(CurrentStation))
	{
		for (AActor* plant : bench->HerbsOnTable) {

			if (ABasePlant* basePlant = Cast<ABasePlant>(plant))
			{
				basePlant->Body->SetSimulatePhysics(true);
			}
			 if (APlantPart* plantPart = Cast<APlantPart>(plant))
			 {
				 plantPart->Body->SetSimulatePhysics(true);
			 }
		}
	}

	if (AAlchemySimulatorCharacter* AlchemyChar = Cast<AAlchemySimulatorCharacter>(GetPawn()))
	{
		AlchemyChar->GetMesh()->SetHiddenInGame(true, true);
	}

	RefreshInputMode();
}

void AAlchemySimulatorPlayerController::HandleWidgetStackChanged(UBaseGameWidget* Widget)
{
	RefreshInputMode();
}

void AAlchemySimulatorPlayerController::RefreshInputMode()
{
	// SetIgnore*Input is counter-based, so reset first — otherwise repeated
	// refreshes (CloseAll broadcasts once per widget) would stack the counters.
	ResetIgnoreLookInput();
	ResetIgnoreMoveInput();

	// Pawn look/move is suppressed in every context except plain gameplay.
	const bool bSuppressPawnInput = true;

	// 1. A minigame is fully modal.
	if (MinigameManager && MinigameManager->GetActiveMinigameWidget())
	{
		FInputModeUIOnly Mode;
		Mode.SetWidgetToFocus(MinigameManager->GetActiveMinigameWidget()->TakeWidget());
		SetInputMode(Mode);
		bShowMouseCursor = true;
		bEnableMouseOverEvents = true;
		bEnableClickEvents = true;
		SetIgnoreLookInput(bSuppressPawnInput);
		SetIgnoreMoveInput(bSuppressPawnInput);
		return;
	}

	// 2. The top-most stack widget owns click priority.
	if (UBaseGameWidget* Top = WidgetManager->GetTopWidget())
	{
		// A modal widget owns the keyboard outright. GameAndUI leaves every
		// Enhanced Input action live (Jump, Interact and Back are not covered
		// by the ignore counters) and lets Slate move focus off the widget the
		// moment anything else is clicked, which strands its key handling.
		if (Top->IsModal())
		{
			FInputModeUIOnly Mode;
			Mode.SetWidgetToFocus(Top->TakeWidget());
			SetInputMode(Mode);
			bShowMouseCursor = true;
			bEnableMouseOverEvents = true;
			bEnableClickEvents = true;
			SetIgnoreLookInput(bSuppressPawnInput);
			SetIgnoreMoveInput(bSuppressPawnInput);
			return;
		}

		FInputModeGameAndUI Mode;
		Mode.SetWidgetToFocus(Top->TakeWidget());
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
		bShowMouseCursor = true;
		bEnableMouseOverEvents = true;
		bEnableClickEvents = true;
		SetIgnoreLookInput(bSuppressPawnInput);
		SetIgnoreMoveInput(bSuppressPawnInput);
		return;
	}

	// 3. At a station with no widget open — world clicks go to table items.
	if (IsAtStation())
	{
		FInputModeGameAndUI Mode;
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
		bShowMouseCursor = true;
		bEnableMouseOverEvents = true;
		bEnableClickEvents = true;
		SetIgnoreLookInput(bSuppressPawnInput);
		SetIgnoreMoveInput(bSuppressPawnInput);
		// No widget to focus, so focus the viewport itself or the first click is eaten.
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
		return;
	}

	// 4. Plain gameplay — pawn input restored by the resets above.
	FInputModeGameOnly Mode;
	Mode.SetConsumeCaptureMouseDown(false);
	SetInputMode(Mode);
	bShowMouseCursor = false;
	bEnableMouseOverEvents = false;
	bEnableClickEvents = false;
}

void AAlchemySimulatorPlayerController::RemoveStationController()
{
	if (!IsAtStation())
	{
		return;
	}

	// A drag must not outlive the station it started at: PlayerTick would keep
	// moving DraggedActor with no workbench left to clamp it to, and its physics
	// would stay disabled forever.
	if (bIsDraggingWorldActor)
	{
		StopWorldDrag();
	}

	// Still needs CurrentStation, so it runs before the clear below.
	ResetActiveTool();

	ABasicInteractableStationObject* Station = CurrentStation;
	// Cleared first, so every RefreshInputMode triggered from here on — CloseAll
	// broadcasts once per popped widget — already sees us out of station mode.
	CurrentStation = nullptr;

	WidgetManager->CloseAll();

	if (ABasicWorkbench* bench = Cast<ABasicWorkbench>(Station))
	{
		for (AActor* plant : bench->HerbsOnTable) {
			if (ABasePlant* basePlant = Cast<ABasePlant>(plant))
			{
				basePlant->Body->SetSimulatePhysics(false);
			}
			if (APlantPart* plantPart = Cast<APlantPart>(plant))
			{
				plantPart->Body->SetSimulatePhysics(false);
			}
		}
	}

	if (AAlchemySimulatorCharacter* AlchemyChar = Cast<AAlchemySimulatorCharacter>(GetPawn()))
	{
		AlchemyChar->GetMesh()->SetHiddenInGame(false, true);
	}

	// Camera: the mirror of the enter half above.
	if (InteractionRig)
	{
		InteractionRig->DisableTilt();
	}
	if (PreStationViewTarget)
	{
		SetViewTargetWithBlend(PreStationViewTarget, 0.35f);
		PreStationViewTarget = nullptr;
	}

	RefreshInputMode();
}

void AAlchemySimulatorPlayerController::PushWidget(UBaseGameWidget* Widget)
{
	WidgetManager->PushWidget(Widget);
	// Disable camera tilt whenever the stack is non-empty
	if (InteractionRig && WidgetManager->HasOpenWidgets())
	{
		InteractionRig->DisableTilt();
	}

	// Input mode is refreshed via the stack's OnWidgetPushed delegate.
}

void AAlchemySimulatorPlayerController::PopWidget()
{
	WidgetManager->PopWidget();
	UE_LOG(LogTemp, Error, TEXT("Popping widget"));
	// Re-enable camera tilt once all widgets are dismissed
	if (InteractionRig && !WidgetManager->HasOpenWidgets())
	{
		InteractionRig->EnableTilt();
	}

	// Input mode is refreshed via the stack's OnWidgetPopped delegate.
}

void AAlchemySimulatorPlayerController::SetActiveTool(ABaseTool* tool)
{
	if (IsAtStation())
	{
		UE_LOG(LogTemp, Error, TEXT("Setting active tool"));

		if (CurrentStation->ActiveToolIndex != -1 && *CurrentStation->Tools.Find(CurrentStation->ActiveToolIndex) == tool)
		{
			ResetActiveTool();
			return;
		}
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

void AAlchemySimulatorPlayerController::RestoreCustomCursor()
{
	if (CursorWidgetInstance)
	{
		SetMouseCursorWidget(EMouseCursor::Custom, CursorWidgetInstance.Get());
		FSlateApplication::Get().QueryCursor();
	}
}

void AAlchemySimulatorPlayerController::ResetActiveTool()
{
	SetMouseCursorWidget(EMouseCursor::Custom, nullptr);
	CurrentMouseCursor = EMouseCursor::Default;

	if (CurrentStation)
	{
		CurrentStation->SetActiveTool(nullptr);
	}
}

void AAlchemySimulatorPlayerController::DoBack()
{
	UE_LOG(LogTemp, Error, TEXT("Backing"));

	if (WidgetManager->HasOpenWidgets())
	{
		// PopWidget refreshes input mode via the stack delegate.
		PopWidget();
		return;
	}

	if (IsAtStation())
	{
		// Camera and tilt are RemoveStationController's job now.
		RemoveStationController();
		return;
	}

	// PushWidget refreshes input mode via the stack delegate.
	PushWidget(CreateWidget<UCharacterScreenWidget>(this, CharacterScreenWidgetClass));

}
void AAlchemySimulatorPlayerController::DebugClick()
{
	if (!IsAtStation()) return;

	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 2.f, 12, FColor::Red, false, 3.f);
		UE_LOG(LogTemp, Warning, TEXT("[DebugClick] actor: %s | component: %s | location: %s"),
			*GetNameSafe(Hit.GetActor()),
			*GetNameSafe(Hit.GetComponent()),
			*Hit.ImpactPoint.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[DebugClick] No hit"));
	}
}

bool AAlchemySimulatorPlayerController::TraceFromScreenPosition(const FVector2D& ScreenPos, FHitResult& OutHit) const
{
	FVector WorldLocation;
	FVector WorldDirection;

	if (!DeprojectScreenPositionToWorld(ScreenPos.X, ScreenPos.Y, WorldLocation, WorldDirection))
	{
		UE_LOG(LogTemp, Error, TEXT("DeprojectScreenPositionToWorld failed"));
		return false;
	}

	const FVector Start = WorldLocation;
	const FVector End = Start + (WorldDirection * 10000.f);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(InventoryDropTrace), false);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	if (!bHit)
	{
		UE_LOG(LogTemp, Warning, TEXT("No world hit from screen position"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("Trace hit actor: %s | comp: %s"),
		*GetNameSafe(OutHit.GetActor()),
		*GetNameSafe(OutHit.GetComponent()));

	return true;
}

bool AAlchemySimulatorPlayerController::TryHandleWorldDropFromScreenPosition(UInvDragOperation* DragOp, const FVector2D& ScreenPos)
{
	if (!DragOp) return false;
	if (DragOp->bDropHandledByUI) return false;

	UE_LOG(LogTemp, Warning, TEXT("Trying to handle world drop from screen position: %s"), *ScreenPos.ToString());

	FHitResult Hit;
	if (!TraceFromScreenPosition(ScreenPos, Hit))
	{
		return false;
	}

	if (UPrimitiveComponent* HitComp = Hit.GetComponent())
	{
		if (HitComp->ComponentHasTag(TEXT("WorkbenchDropZone")))
		{
			if (ABasicWorkbench* Workbench = Cast<ABasicWorkbench>(HitComp->GetOwner()))
			{
				return Workbench->TryPlaceDraggedItem(DragOp, Hit);
			}
		}
	}

	if (ABasicWorkbench* Workbench = Cast<ABasicWorkbench>(Hit.GetActor()))
	{
		return Workbench->TryPlaceDraggedItem(DragOp, Hit);
	}

	return false;
}

void AAlchemySimulatorPlayerController::StartWorldDrag(AActor* ActorToDrag)
{

	UE_LOG(LogTemp, Warning, TEXT("Starting world drag for actor: %s"), *GetNameSafe(ActorToDrag));
	if (!ActorToDrag)
	{
		return;
	}

	DraggedActor = ActorToDrag;
	bIsDraggingWorldActor = true;

	DragPlaneOrigin = ActorToDrag->GetActorLocation();
	DragPlaneNormal = FVector::UpVector;

	FHitResult Hit;
	if (GetHitResultUnderCursor(ECC_Visibility, false, Hit))
	{
		DragOffset = ActorToDrag->GetActorLocation() - Hit.Location;
	}
	else
	{
		DragOffset = FVector::ZeroVector;
	}

	// Disable physics on every simulated component so it doesn't fight SetActorLocation
	DraggedPhysicsComponents.Empty();
	TArray<UPrimitiveComponent*> PrimComps;
	ActorToDrag->GetComponents<UPrimitiveComponent>(PrimComps);
	for (UPrimitiveComponent* Comp : PrimComps)
	{
		if (Comp && Comp->IsSimulatingPhysics())
		{
			Comp->SetSimulatePhysics(false);
			DraggedPhysicsComponents.Add(Comp);
		}
	}

	DragSmoothedLocation = ActorToDrag->GetActorLocation();
	bShowMouseCursor = true;
}

void AAlchemySimulatorPlayerController::StopWorldDrag()
{
	for (UPrimitiveComponent* Comp : DraggedPhysicsComponents)
	{
		if (Comp)
		{
			Comp->SetSimulatePhysics(true);
		}
	}
	DraggedPhysicsComponents.Empty();

	bIsDraggingWorldActor = false;
	DraggedActor = nullptr;
	DragOffset = FVector::ZeroVector;
}

void AAlchemySimulatorPlayerController::RotateDraggedItem(const FInputActionValue& Value)
{
	if (!bIsDraggingWorldActor || !DraggedActor)
	{
		return;
	}

	const float AxisValue = Value.Get<float>();

	if (FMath::IsNearlyZero(AxisValue))
	{
		return;
	}

	const float YawDelta = AxisValue * RotationSpeed * GetWorld()->GetDeltaSeconds();

	// The grab point is the mouse cursor projected onto the drag plane.
	// DragOffset is always the vector from that point to the actor pivot,
	// so subtracting it recovers the grab point.
	const FVector GrabPoint = DragSmoothedLocation - DragOffset;

	// Orbit the offset vector around the grab point (yaw only — no tilt)
	const FVector RotatedOffset = DragOffset.RotateAngleAxis(YawDelta, FVector::UpVector);
	DragOffset = RotatedOffset;

	// Place the actor at the new orbited position and spin it the same amount
	const FVector NewLocation = GrabPoint + RotatedOffset;
	FRotator NewRotation = DraggedActor->GetActorRotation();
	NewRotation.Yaw += YawDelta;

	DraggedActor->SetActorLocationAndRotation(NewLocation, NewRotation);
	DragSmoothedLocation = NewLocation;
}

void AAlchemySimulatorPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (!bIsDraggingWorldActor || !DraggedActor)
	{
		return;
	}

	FVector WorldOrigin;
	FVector WorldDirection;

	if (!DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
	{
		return;
	}

	// Vytvoríme dlhú ray čiaru z kamery cez kurzor
	const FVector TraceEnd = WorldOrigin + WorldDirection * 100000.0f;

	// Nájdeme priesečník ray-u s horizontálnou rovinou
	const FPlane DragPlane(DragPlaneOrigin, DragPlaneNormal);
	const FVector NewLocation = FMath::LinePlaneIntersection(
		WorldOrigin,
		TraceEnd,
		DragPlane
	);

	FVector TargetLocation = NewLocation + DragOffset;
	TargetLocation.Z += DragZLift;
	if (CurrentStation)
	{
		ABasicWorkbench* CurrentWorkbench = Cast<ABasicWorkbench>(CurrentStation);
		if (CurrentWorkbench)
		{
			TargetLocation = CurrentWorkbench->ClampActorToWorkbench(DraggedActor, TargetLocation);
		}
	}

	// Smooth the position to eliminate mouse jitter; speed of 25 keeps it responsive
	DragSmoothedLocation = FMath::VInterpTo(DragSmoothedLocation, TargetLocation, DeltaTime, 25.0f);
	DraggedActor->SetActorLocation(DragSmoothedLocation);
}

void AAlchemySimulatorPlayerController::StopLeftMouseAction()
{
	if (bIsDraggingWorldActor)
	{
		StopWorldDrag();
	}
}


void AAlchemySimulatorPlayerController::HandleDialogueStarted(ANPCCharacter* NPC)
{
	UE_LOG(LogTemp, Warning, TEXT("Handling dialogue started in Controller with NPC: %s"), *GetNameSafe(NPC));
    if (!DialogueWidgetClass) return;

    UDialogueWidget* W = CreateWidget<UDialogueWidget>(this, DialogueWidgetClass);
    if (!W) return;

    W->Setup(DialogueRuntime, NPC);
    PushWidget(W);
}
