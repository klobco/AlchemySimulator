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
#include "InteractionCameraRig.h"
#include "BasicInteractableStationObject.h"
#include "GameFramework/Character.h"
#include "BasePlant.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InvDragOperation.h"
#include "BasicWorkbench.h"
#include "CustomCursorWidget.h"
#include "BaseTool.h"
#include "ItemDefinitionBase.h"
#include "BaseGameWidget.h"
#include "WidgetStackManager.h"
#include "DrawDebugHelpers.h"

AAlchemySimulatorPlayerController::AAlchemySimulatorPlayerController()
{
	WidgetManager = CreateDefaultSubobject<UWidgetStackManager>(TEXT("WidgetManager"));
}

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

	if (CurrentTarget != nullptr)
	{
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
				IInteractable::Execute_Interact(CurrentTarget.GetObject(), GetPawn());
			}
			else
			{
				IInteractable::Execute_Interact(CurrentTarget.GetObject(), GetPawn());
			}
		}
		else
		{
			WidgetManager->CloseAll();
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

void AAlchemySimulatorPlayerController::SetupStationController(ABasicInteractableStationObject* station)
{
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

void AAlchemySimulatorPlayerController::RemoveStationController()
{
	ResetActiveTool();
	WidgetManager->CloseAll();
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

void AAlchemySimulatorPlayerController::PushWidget(UBaseGameWidget* Widget)
{
	WidgetManager->PushWidget(Widget);
	// Disable camera tilt whenever the stack is non-empty
	if (InteractionRig && WidgetManager->HasOpenWidgets())
	{
		InteractionRig->DisableTilt();
	}
}

void AAlchemySimulatorPlayerController::PopWidget()
{
	WidgetManager->PopWidget();
	// Re-enable camera tilt once all widgets are dismissed
	if (InteractionRig && !WidgetManager->HasOpenWidgets())
	{
		InteractionRig->EnableTilt();
	}
}

void AAlchemySimulatorPlayerController::SetActiveTool(ABaseTool* tool)
{
	if (Interacting && CurrentStation)
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
		PopWidget();
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
void AAlchemySimulatorPlayerController::DebugClick()
{
	if (!Interacting) return;

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

	// Rovina dragovania bude vo výške aktuálnej rastliny.
	// Čiže rastlina sa bude hýbať po horizontálnej rovine.
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

	bShowMouseCursor = true;
}

void AAlchemySimulatorPlayerController::StopWorldDrag()
{
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

	const float DeltaSeconds = GetWorld()->GetDeltaSeconds();

	FRotator CurrentRotation = DraggedActor->GetActorRotation();
	CurrentRotation.Yaw += AxisValue * RotationSpeed * DeltaSeconds;

	DraggedActor->SetActorRotation(CurrentRotation);
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
	if (CurrentStation)
	{
		ABasicWorkbench* CurrentWorkbench = Cast<ABasicWorkbench>(CurrentStation);
		if (CurrentWorkbench)
		{
			TargetLocation = CurrentWorkbench->ClampLocationToWorkbench(TargetLocation);
		}
	}

	DraggedActor->SetActorLocation(TargetLocation);
}

void AAlchemySimulatorPlayerController::StopLeftMouseAction()
{
	if (bIsDraggingWorldActor)
	{
		StopWorldDrag();
	}
}