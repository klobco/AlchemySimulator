// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerModes/PlayerModeBase.h"
#include "ExplorationPlayerMode.generated.h"

/**
 * Walking around: cursor hidden, pawn input live, interaction detector running.
 *
 * This is the bottom of the mode stack and is never popped — "no mode" is not a
 * state the controller can be in, so RefreshInputMode always has something to ask.
 */
UCLASS()
class ALCHEMYSIMULATOR_API UExplorationPlayerMode : public UPlayerModeBase
{
	GENERATED_BODY()

public:
	UExplorationPlayerMode();

	virtual FPlayerModeInputSpec GetInputSpec() const override;
	virtual FName GetModeName() const override { return TEXT("Exploration"); }

	/** Escape with nothing else open opens the character screen. */
	virtual bool HandleBackAction() override;
};
