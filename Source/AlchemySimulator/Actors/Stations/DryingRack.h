// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/Stations/BasicInteractableStationObject.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Subsystems/GameTimeSubsystem.h"
#include "DryingRack.generated.h"

/** A hook's item crossed into a new drying stage. For VFX/audio; the stage itself stays derived. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnDryingStageChanged,
	int32, SlotIndex,
	FGameplayTag, NewStageTag
);

/**
 * A four-hook rack that dries whatever hangs on it as game time passes.
 *
 * The rack owns no drying state of its own beyond "when did I last look at the clock". Each
 * item's progress lives on its own FItemInstanceData::DryingMinutes, so a half-dried leaf keeps
 * its progress through rack -> bag -> table and back, and the stage is always derived from that
 * one number (see UDataAssetDryingProfile).
 */
UCLASS()
class ALCHEMYSIMULATOR_API ADryingRack : public ABasicInteractableStationObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Category = "components")
	class UInventoryComponent* DryingRackInventory;

	/**
	 * Scales how fast this rack dries: 1.0 means an item takes exactly its MinutesToFullyDry.
	 *
	 * An inline initialiser, not a constructor assignment -- a Blueprint CDO's values are
	 * serialised after the constructor runs, which is what once made ToolsLimit read 0.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Drying", meta = (ClampMin = "0.0"))
	float DryingRateMultiplier = 1.0f;

	UPROPERTY(BlueprintAssignable, Category = "Drying")
	FOnDryingStageChanged OnDryingStageChanged;

	ADryingRack();

	virtual void Interact_Implementation(APawn* By) override;

	virtual UInventoryComponent* GetStationInventory() const override;

protected:

	virtual void BeginPlay() override;

    UFUNCTION()
    void HandleRackInventoryChanged(EInventoryChangeType ChangeType, int32 SlotIndex);

    UFUNCTION()
    void HandleGameMinute(const FAlchemyGameTime& NewTime);

    /**
     * Advance one hook by ElapsedMinutes, committing the processing method of every stage the
     * item crosses on the way -- so one AddMinutes(600) that jumps Wet straight past Dry still
     * leaves Processing.Dried on the item, exactly as a slow dry would have.
     */
    void AdvanceDrying(int32 SlotIndex, float ElapsedMinutes);

    /** Pull a slot's DryingMinutes back to its profile's cap, for an item that arrives beyond it. */
    void ClampDryingToProfile(int32 SlotIndex);

    /**
     * Absolute game-minute stamp of the last drying update. Initialised from the clock in
     * BeginPlay and never from 0: the clock starts at day one 06:00, so a zero stamp would hand
     * every hook six game hours of drying on the first broadcast.
     */
    int64 LastDryingStamp = 0;
};
