// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerModes/ExplorationPlayerMode.h"
#include "AlchemySimulatorPlayerController.h"
#include "InputAction.h"

UExplorationPlayerMode::UExplorationPlayerMode()
{
	// Locomotion dies while a widget or minigame is up. Soft paths, so a moved
	// or renamed asset logs a warning instead of failing the constructor —
	// see PlayerModeBase.cpp's ResolveSoftList.
	BlockedActionsWhileUIOpen = {
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_Jump.IA_Jump"))),
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_Move.IA_Move"))),
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_Look.IA_Look"))),
		TSoftObjectPtr<UInputAction>(FSoftObjectPath(TEXT("/Game/Input/Actions/IA_MouseLook.IA_MouseLook")))
	};
}

FPlayerModeInputSpec UExplorationPlayerMode::GetInputSpec() const
{
	FPlayerModeInputSpec Spec;
	Spec.Kind = EPlayerInputModeKind::GameOnly;
	Spec.bShowCursor = false;
	Spec.bSuppressPawnInput = false;
	return Spec;
}

bool UExplorationPlayerMode::HandleBackAction()
{
	AAlchemySimulatorPlayerController* PC = GetPlayerController();
	if (!PC)
	{
		return false;
	}

	PC->OpenCharacterScreen();
	return true;
}
