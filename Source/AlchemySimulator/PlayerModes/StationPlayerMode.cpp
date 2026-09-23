// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerModes/StationPlayerMode.h"
#include "AlchemySimulatorPlayerController.h"
#include "AlchemySimulatorCharacter.h"
#include "InteractionCameraRig.h"
#include "Actors/Stations/BasicInteractableStationObject.h"
#include "Actors/Stations/BasicWorkbench.h"
#include "Actors/Plants/BasePlant.h"
#include "Actors/Plants/PlantPart.h"
#include "Widgets/WidgetStackManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "InputAction.h"

UStationPlayerMode::UStationPlayerMode()
{
	// Locomotion is meaningless at a table and must be gated in the mapping
	// context, not by the ignore counters — those never covered Jump, which is
	// bound straight to ACharacter::Jump.
	// Interact, Back, LeftMouse and RotateDraggedItem deliberately stay mapped.
	BlockedActions = {
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"))),
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_Move.IA_Move"))),
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_Look.IA_Look"))),
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook")))
	};
}

void UStationPlayerMode::EnterMode()
{
	AAlchemySimulatorPlayerController* PC = GetPlayerController();
	if (!PC || !Station)
	{
		return;
	}

	// Camera: frame the station and remember where to blend back to.
	PreStationViewTarget = PC->GetViewTarget();
	if (PC->InteractionRig && Station->InteractionViewPoint)
	{
		PC->InteractionRig->SetActorLocation(Station->InteractionViewPoint->GetComponentLocation());
		PC->InteractionRig->SetActorRotation(Station->InteractionViewPoint->GetComponentRotation());
		PC->SetViewTargetWithBlend(PC->InteractionRig, 0.35f);
		PC->InteractionRig->EnableTilt();
	}

	SetTablePhysics(true);

	if (AAlchemySimulatorCharacter* AlchemyChar = Cast<AAlchemySimulatorCharacter>(PC->GetPawn()))
	{
		AlchemyChar->GetMesh()->SetHiddenInGame(true, true);
	}
}

void UStationPlayerMode::ExitMode()
{
	AAlchemySimulatorPlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return;
	}

	// A drag must not outlive the station it started at: PlayerTick would keep
	// moving DraggedActor with no workbench left to clamp it to, and its physics
	// would stay disabled forever.
	PC->StopWorldDrag();

	PC->ResetActiveTool();

	// By the time ExitMode runs the mode is already off the stack, so the
	// refreshes CloseAll triggers (one per popped widget) see us out of station mode.
	if (PC->WidgetManager)
	{
		PC->WidgetManager->CloseAll();
	}

	SetTablePhysics(false);

	if (AAlchemySimulatorCharacter* AlchemyChar = Cast<AAlchemySimulatorCharacter>(PC->GetPawn()))
	{
		AlchemyChar->GetMesh()->SetHiddenInGame(false, true);
	}

	// Camera: the mirror of EnterMode.
	if (PC->InteractionRig)
	{
		PC->InteractionRig->DisableTilt();
	}
	if (PreStationViewTarget)
	{
		PC->SetViewTargetWithBlend(PreStationViewTarget, 0.35f);
		PreStationViewTarget = nullptr;
	}
}

FPlayerModeInputSpec UStationPlayerMode::GetInputSpec() const
{
	FPlayerModeInputSpec Spec;
	Spec.Kind = EPlayerInputModeKind::GameAndUI;
	Spec.bShowCursor = true;
	Spec.bSuppressPawnInput = true;
	// No widget to focus, so focus the viewport itself or the first click is eaten.
	Spec.bFocusGameViewport = true;
	return Spec;
}

bool UStationPlayerMode::HandleBackAction()
{
	AAlchemySimulatorPlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return false;
	}

	// Pops this mode, which runs ExitMode.
	PC->PopPlayerMode(this);
	return true;
}

void UStationPlayerMode::SetTablePhysics(bool bSimulate) const
{
	ABasicWorkbench* Bench = Cast<ABasicWorkbench>(Station);
	if (!Bench)
	{
		return;
	}

	for (AActor* Item : Bench->HerbsOnTable)
	{
		if (ABasePlant* BasePlant = Cast<ABasePlant>(Item))
		{
			BasePlant->Body->SetSimulatePhysics(bSimulate);
		}
		if (APlantPart* PlantPart = Cast<APlantPart>(Item))
		{
			PlantPart->Body->SetSimulatePhysics(bSimulate);
		}
	}
}
