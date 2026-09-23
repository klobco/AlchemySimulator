// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IInteractable.h"
#include "InteractionDetectorComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
	FOnFocusedInteractableChanged,
	UObject*, NewTarget,
	UObject*, OldTarget
);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ALCHEMYSIMULATOR_API UInteractionDetectorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractionDetectorComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	float Radius = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	float UpdateRateHz = 10.f; 

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	TEnumAsByte<ECollisionChannel> LineOfSightChannel = ECC_Visibility;

	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	FName RequiredActorTag = FName("Interactable");

	UPROPERTY(BlueprintAssignable)
	FOnFocusedInteractableChanged OnFocusedChanged;

	UFUNCTION(BlueprintCallable, Category = "Interact")
	AActor* GetCurrent() const
	{
		AActor* Ptr = Current.Get();
		return IsValid(Ptr) ? Ptr : nullptr;
	}

	UFUNCTION(BlueprintCallable, Category = "Interact")
	void TryInteract(APawn* By);

	/**
	 * Start/stop scanning. Disabling drops the current focus (firing OnFocEnd,
	 * so highlights clear) and stops the timer; enabling re-scans immediately.
	 *
	 * Player modes drive this — a mode that does not want focus targeting, such
	 * as standing at a station, would otherwise keep re-scoring while the player
	 * stands still and drift focus onto whatever is on the table.
	 */
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void SetDetectionEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Interact")
	bool IsDetectionEnabled() const { return bDetectionEnabled; }

protected:
	virtual void BeginPlay() override;

private:
	FTimerHandle Timer;
	TWeakObjectPtr<AActor> Current;
	bool bDetectionEnabled = true;

	void TickDetect();

	/** Drop the focused target, notifying listeners as a normal focus change would. */
	void ClearCurrent();
	bool PassesLOS(const FVector& Eye, const FVector& Target) const;
	float ScoreCandidate(AActor* Actor, const FVector& Eye, const FVector& Fwd) const;
};
