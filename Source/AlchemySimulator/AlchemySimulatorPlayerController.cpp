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
#include "PlayerModes/PlayerModeBase.h"
#include "PlayerModes/ExplorationPlayerMode.h"
#include "PlayerModes/StationPlayerMode.h"
#include "InputAction.h"

AAlchemySimulatorPlayerController::AAlchemySimulatorPlayerController()
{
	WidgetManager = CreateDefaultSubobject<UWidgetStackManager>(TEXT("WidgetManager"));
	DialogueRuntime = CreateDefaultSubobject<UDialogueRuntimeComponent>(TEXT("DialogueRuntime"));
	MinigameManager = CreateDefaultSubobject<UMinigameManagerComponent>(TEXT("MinigameManager"));

	ExplorationModeClass = UExplorationPlayerMode::StaticClass();
	StationModeClass = UStationPlayerMode::StaticClass();
}

void AAlchemySimulatorPlayerController::BeginPlay()
{
	UE_LOG(LogAlchemySimulator, Error, TEXT("Using my custom controller"));
	Super::BeginPlay();

	if (GetPawn())
	{
		BindToDetector(GetPawn());
	}

	// Before anything can refresh the input mode: the stack must never be empty.
	InitializePlayerModeStack();

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

	// Mapping contexts are not added here any more: the active player mode owns
	// which contexts are installed, so adding them here as well would leave a
	// copy behind that no mode can remove. BeginPlay's first PushPlayerMode
	// installs them via ApplyModeMappingContexts.

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
		// A detector bound after the stack was built (a re-possess) starts
		// enabled, which may not be what the active mode wants.
		ApplyActiveModeState();
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

	// Captured up front: entering a mode can switch the interaction detector off,
	// which clears CurrentTarget mid-call. Everything below wants the target we
	// interacted with, not whatever focus happens to be by then.
	UObject* Target = CurrentTarget.GetObject();
	if (Target == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Interacting with null"));
		return;
	}

	if (ABasicInteractableStationObject* station = Cast<ABasicInteractableStationObject>(Target))
	{
		// Every side effect of entering (view target, tilt, physics, mesh, input
		// mode) belongs to the station mode, so the exit stays a mirror.
		SetupStationController(station);
		IInteractable::Execute_Interact(Target, GetPawn());
	}
	else if (ANPCCharacter* NPC = Cast<ANPCCharacter>(Target))
	{
		// Starting only. UDialogueWidget is modal, so once it is up this
		// action cannot fire again - ending the conversation is the widget's
		// own exit key. StartDialogue self-guards on IsInDialogue().
		DialogueRuntime->StartDialogue(NPC, NPC->DialogueProviderClass);
	}
	else
	{
		IInteractable::Execute_Interact(Target, GetPawn());
	}
}

void AAlchemySimulatorPlayerController::InitializePlayerModeStack()
{
	if (ModeStack.Num() > 0)
	{
		return;
	}

	UClass* ModeClass = ExplorationModeClass.Get();
	if (!ModeClass)
	{
		ModeClass = UExplorationPlayerMode::StaticClass();
	}

	PushPlayerMode(NewObject<UExplorationPlayerMode>(this, ModeClass));
}

UPlayerModeBase* AAlchemySimulatorPlayerController::GetActivePlayerMode() const
{
	return ModeStack.Num() > 0 ? ModeStack.Last() : nullptr;
}

UPlayerModeBase* AAlchemySimulatorPlayerController::FindPlayerMode(TSubclassOf<UPlayerModeBase> ModeClass) const
{
	if (!ModeClass)
	{
		return nullptr;
	}

	for (int32 i = ModeStack.Num() - 1; i >= 0; --i)
	{
		if (ModeStack[i] && ModeStack[i]->IsA(ModeClass.Get()))
		{
			return ModeStack[i];
		}
	}

	return nullptr;
}

ABasicInteractableStationObject* AAlchemySimulatorPlayerController::GetCurrentStation() const
{
	// The station mode *is* the station state — there is no second copy.
	if (UStationPlayerMode* StationMode = Cast<UStationPlayerMode>(FindPlayerMode(UStationPlayerMode::StaticClass())))
	{
		return StationMode->Station;
	}

	return nullptr;
}

void AAlchemySimulatorPlayerController::PushPlayerMode(UPlayerModeBase* Mode)
{
	if (!Mode)
	{
		return;
	}

	Mode->OwningController = this;
	// On the stack before EnterMode runs, so anything EnterMode triggers already
	// sees the new mode as active.
	ModeStack.Add(Mode);

	UE_LOG(LogTemp, Warning, TEXT("[PlayerMode] push %s (depth %d)"), *Mode->GetModeName().ToString(), ModeStack.Num());

	Mode->EnterMode();

	ApplyActiveModeState();
	RefreshInputMode();
}

void AAlchemySimulatorPlayerController::PopPlayerMode(UPlayerModeBase* Mode)
{
	if (!Mode)
	{
		return;
	}

	const int32 Index = ModeStack.IndexOfByKey(Mode);
	if (Index == INDEX_NONE)
	{
		return;
	}

	// Exploration is the floor: popping it would leave RefreshInputMode with
	// nothing to ask.
	if (Index == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlayerMode] refused to pop the base mode %s"), *Mode->GetModeName().ToString());
		return;
	}

	// Off the stack before ExitMode runs — ExitMode closes widgets, and each
	// pop refreshes the input mode, which must not still see this mode.
	ModeStack.RemoveAt(Index);

	UE_LOG(LogTemp, Warning, TEXT("[PlayerMode] pop %s (depth %d)"), *Mode->GetModeName().ToString(), ModeStack.Num());

	Mode->ExitMode();

	ApplyActiveModeState();
	RefreshInputMode();
}

void AAlchemySimulatorPlayerController::ApplyActiveModeState()
{
	UPlayerModeBase* Active = GetActivePlayerMode();
	if (!Active)
	{
		return;
	}

	// Focus detection is per-mode: at a station the detector would keep
	// re-scoring while the player stands still and drift focus onto table items.
	if (Detector)
	{
		Detector->SetDetectionEnabled(Active->WantsInteractionDetector());
	}

	ApplyModeMappingContexts();
}

void AAlchemySimulatorPlayerController::ApplyModeMappingContexts()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (!Subsystem)
	{
		return;
	}

	UPlayerModeBase* Active = GetActivePlayerMode();
	if (!Active)
	{
		return;
	}

	// A widget or minigame owning the screen is part of what decides which keys
	// are live, so it is part of the signature below.
	const bool bUIOpen =
		(MinigameManager && MinigameManager->GetActiveMinigameWidget() != nullptr) ||
		(WidgetManager && WidgetManager->HasOpenWidgets());

	// CloseAll refreshes once per popped widget; without this the whole set
	// would be torn down and rebuilt each time.
	if (Active == LastMappingMode && bUIOpen == bLastMappingUIOpen && AppliedMappingContexts.Num() > 0)
	{
		return;
	}
	LastMappingMode = Active;
	bLastMappingUIOpen = bUIOpen;

	for (UInputMappingContext* Applied : AppliedMappingContexts)
	{
		if (Applied)
		{
			Subsystem->RemoveMappingContext(Applied);
		}
	}
	AppliedMappingContexts.Reset();

	// A mode names contexts only when it needs different bindings; otherwise it
	// takes the controller's default set.
	TArray<const UInputMappingContext*> SourceContexts;
	Active->GetMappingContexts(SourceContexts);
	if (SourceContexts.Num() == 0)
	{
		for (UInputMappingContext* Context : DefaultMappingContexts)
		{
			if (Context)
			{
				SourceContexts.Add(Context);
			}
		}

		if (!ShouldUseTouchControls())
		{
			for (UInputMappingContext* Context : MobileExcludedMappingContexts)
			{
				if (Context)
				{
					SourceContexts.Add(Context);
				}
			}
		}
	}

	TArray<const UInputAction*> BlockedActions;
	Active->GetBlockedActions(bUIOpen, BlockedActions);

	for (const UInputMappingContext* Source : SourceContexts)
	{
		UInputMappingContext* ToInstall = const_cast<UInputMappingContext*>(Source);

		// Does this context actually map anything we are blocking? Most do not,
		// and an untouched context is installed as-is.
		bool bNeedsFiltering = false;
		for (const FEnhancedActionKeyMapping& Mapping : Source->GetMappings())
		{
			if (BlockedActions.Contains(Mapping.Action))
			{
				bNeedsFiltering = true;
				break;
			}
		}

		if (bNeedsFiltering)
		{
			// Duplicate and unmap rather than rebuild mapping-by-mapping:
			// MapKey would drop the modifiers and triggers (WASD swizzle/negate,
			// rotate negation) that make the bindings work.
			UInputMappingContext* Filtered = DuplicateObject<UInputMappingContext>(Source, this);
			for (const UInputAction* Blocked : BlockedActions)
			{
				Filtered->UnmapAllKeysFromAction(Blocked);
			}
			ToInstall = Filtered;
		}

		Subsystem->AddMappingContext(ToInstall, Active->MappingPriority);
		AppliedMappingContexts.Add(ToInstall);
	}

	UE_LOG(LogTemp, Warning, TEXT("[PlayerMode] contexts for %s: %d installed, %d action(s) blocked (UI open: %s)"),
		*Active->GetModeName().ToString(), AppliedMappingContexts.Num(), BlockedActions.Num(),
		bUIOpen ? TEXT("yes") : TEXT("no"));
}

void AAlchemySimulatorPlayerController::SetupStationController(ABasicInteractableStationObject* station)
{
	if (!station || IsAtStation())
	{
		return;
	}

	UClass* ModeClass = StationModeClass.Get();
	if (!ModeClass)
	{
		ModeClass = UStationPlayerMode::StaticClass();
	}

	UStationPlayerMode* Mode = NewObject<UStationPlayerMode>(this, ModeClass);
	// Configure before pushing — EnterMode needs the station.
	Mode->Station = station;
	PushPlayerMode(Mode);
}

void AAlchemySimulatorPlayerController::HandleWidgetStackChanged(UBaseGameWidget* Widget)
{
	RefreshInputMode();
}

void AAlchemySimulatorPlayerController::RefreshInputMode()
{
	UUserWidget* ActiveMinigame = MinigameManager ? MinigameManager->GetActiveMinigameWidget() : nullptr;
	UBaseGameWidget* TopWidget = WidgetManager ? WidgetManager->GetTopWidget() : nullptr;

	// A world drag is only tenable while the world owns the mouse. Under a
	// widget or a minigame the LeftMouseAction Completed event never arrives, so
	// the drag would never end: physics stays off on DraggedActor and PlayerTick
	// keeps moving it for the rest of the session.
	if (bIsDraggingWorldActor && (ActiveMinigame || TopWidget))
	{
		StopWorldDrag();
	}

	// Which keys are live depends on the mode *and* on whether UI owns the
	// screen, so it is refreshed here too, not only on mode changes. Free when
	// nothing has changed.
	ApplyModeMappingContexts();

	// 1. A minigame is fully modal.
	if (ActiveMinigame)
	{
		FPlayerModeInputSpec Spec;
		Spec.Kind = EPlayerInputModeKind::UIOnly;
		Spec.bShowCursor = true;
		Spec.WidgetToFocus = ActiveMinigame;
		ApplyInputSpec(Spec);
		return;
	}

	// 2. The top-most stack widget owns click priority.
	if (TopWidget)
	{
		FPlayerModeInputSpec Spec;
		// A modal widget owns the keyboard outright. GameAndUI leaves every
		// Enhanced Input action live (Jump, Interact and Back are not covered
		// by the ignore counters) and lets Slate move focus off the widget the
		// moment anything else is clicked, which strands its key handling.
		Spec.Kind = TopWidget->IsModal() ? EPlayerInputModeKind::UIOnly : EPlayerInputModeKind::GameAndUI;
		Spec.bShowCursor = true;
		Spec.WidgetToFocus = TopWidget;
		ApplyInputSpec(Spec);
		return;
	}

	// 3. Whatever the active player mode asks for. Station and exploration are
	// modes now, not branches; the stack is never empty after BeginPlay.
	if (UPlayerModeBase* ActiveMode = GetActivePlayerMode())
	{
		ApplyInputSpec(ActiveMode->GetInputSpec());
		return;
	}

	// Only reachable before the stack is initialised.
	ApplyInputSpec(FPlayerModeInputSpec());
}

void AAlchemySimulatorPlayerController::ApplyInputSpec(const FPlayerModeInputSpec& Spec)
{
	// SetIgnore*Input is counter-based, so reset first — otherwise repeated
	// refreshes (CloseAll broadcasts once per widget) would stack the counters.
	ResetIgnoreLookInput();
	ResetIgnoreMoveInput();

	switch (Spec.Kind)
	{
	case EPlayerInputModeKind::UIOnly:
	{
		FInputModeUIOnly Mode;
		if (UUserWidget* Focus = Spec.WidgetToFocus.Get())
		{
			Mode.SetWidgetToFocus(Focus->TakeWidget());
		}
		SetInputMode(Mode);
		break;
	}

	case EPlayerInputModeKind::GameAndUI:
	{
		FInputModeGameAndUI Mode;
		if (UUserWidget* Focus = Spec.WidgetToFocus.Get())
		{
			Mode.SetWidgetToFocus(Focus->TakeWidget());
		}
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		// Defaults to true, which hides and re-centers the OS cursor for the
		// whole click-drag gesture and freezes DeprojectMousePositionToWorld.
		Mode.SetHideCursorDuringCapture(false);
		SetInputMode(Mode);
		break;
	}

	case EPlayerInputModeKind::GameOnly:
	default:
	{
		FInputModeGameOnly Mode;
		Mode.SetConsumeCaptureMouseDown(false);
		SetInputMode(Mode);
		break;
	}
	}

	bShowMouseCursor = Spec.bShowCursor;
	bEnableMouseOverEvents = Spec.bShowCursor;
	bEnableClickEvents = Spec.bShowCursor;

	SetIgnoreLookInput(Spec.bSuppressPawnInput);
	SetIgnoreMoveInput(Spec.bSuppressPawnInput);

	if (Spec.bFocusGameViewport && !Spec.WidgetToFocus.IsValid())
	{
		// With nothing focused, Slate eats the first world click as a focus change.
		FSlateApplication::Get().SetAllUserFocusToGameViewport();
	}
}

void AAlchemySimulatorPlayerController::RemoveStationController()
{
	// The mode owns the whole teardown; popping it runs ExitMode.
	PopPlayerMode(FindPlayerMode(UStationPlayerMode::StaticClass()));
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
	ABasicInteractableStationObject* Station = GetCurrentStation();
	if (!Station)
	{
		// No station means no tool to make active — and nothing that would ever
		// clear the custom cursor below, so do not install one either.
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Setting active tool"));

	// Find, not Find-and-deref: ActiveToolIndex != -1 does not guarantee the map
	// still holds that index (a removed tool leaves the index stale), and
	// dereferencing the null result is a crash.
	if (ABaseTool** ActiveTool = Station->Tools.Find(Station->ActiveToolIndex))
	{
		if (*ActiveTool == tool)
		{
			// Clicking the active tool again toggles it off.
			ResetActiveTool();
			return;
		}
	}

	Station->SetActiveTool(tool);

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

	if (ABasicInteractableStationObject* Station = GetCurrentStation())
	{
		Station->SetActiveTool(nullptr);
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

	// Otherwise the active mode decides. Escape used to fall through to "open
	// the character screen" in any mode that was not a station — a new mode had
	// to remember to add a branch here to avoid that.
	if (UPlayerModeBase* Active = GetActivePlayerMode())
	{
		if (Active->HandleBackAction())
		{
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[PlayerMode] Back unhandled by %s"),
		*GetNameSafe(GetActivePlayerMode()));
}

void AAlchemySimulatorPlayerController::OpenCharacterScreen()
{
	if (!CharacterScreenWidgetClass)
	{
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
	if (ABasicWorkbench* CurrentWorkbench = Cast<ABasicWorkbench>(GetCurrentStation()))
	{
		TargetLocation = CurrentWorkbench->ClampActorToWorkbench(DraggedActor, TargetLocation);
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
