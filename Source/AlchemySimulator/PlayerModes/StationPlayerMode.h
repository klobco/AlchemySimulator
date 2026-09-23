// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerModes/PlayerModeBase.h"
#include "StationPlayerMode.generated.h"

class ABasicInteractableStationObject;

/**
 * Standing at a workbench or other station: the camera sits on the station's
 * view point, table items simulate physics, the pawn mesh is hidden and world
 * clicks reach table items.
 *
 * This mode *is* the station state — AAlchemySimulatorPlayerController::GetCurrentStation
 * reads the station off it, so there is no second copy to keep in step.
 * The station must be set before the mode is pushed.
 */
UCLASS()
class ALCHEMYSIMULATOR_API UStationPlayerMode : public UPlayerModeBase
{
	GENERATED_BODY()

public:
	UStationPlayerMode();

	virtual void EnterMode() override;
	virtual void ExitMode() override;
	virtual FPlayerModeInputSpec GetInputSpec() const override;
	virtual FName GetModeName() const override { return TEXT("Station"); }

	/** Escape with nothing else open leaves the station. */
	virtual bool HandleBackAction() override;

	/**
	 * The detector keeps re-scoring while the player stands still at a table,
	 * so it would drift focus onto table items and fire their highlights. There
	 * is nothing to focus at a station — leaving is decided by IsAtStation().
	 */
	virtual bool WantsInteractionDetector() const override { return false; }

	/** Set by the caller before PushPlayerMode. */
	UPROPERTY(BlueprintReadOnly, Category = "Station")
	TObjectPtr<ABasicInteractableStationObject> Station = nullptr;

private:
	/** Where the camera came from, so ExitMode can blend back to it. */
	UPROPERTY()
	TObjectPtr<AActor> PreStationViewTarget = nullptr;

	void SetTablePhysics(bool bSimulate) const;
};
