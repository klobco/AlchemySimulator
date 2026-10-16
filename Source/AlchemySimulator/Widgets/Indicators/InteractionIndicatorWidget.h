// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionIndicatorWidget.generated.h"

/**
 * World-anchored ring drawn over the interaction detector's focused target.
 *
 * A plain viewport widget, deliberately not a UBaseGameWidget: it is HUD, never
 * modal, and must stay off the widget stack so it cannot affect the input mode.
 * It does not own its own visibility either - see RepositionIndicator.
 */
UCLASS()
class ALCHEMYSIMULATOR_API UInteractionIndicatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* OuterCircle;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "InteractionIndicator")
	FVector2D Offset = FVector2D(10.f, 10.f);

	/**
	 * Move the ring onto a world location and scale it for distance.
	 *
	 * Returns false when the location does not project onto the screen (the
	 * target is behind the camera). Visibility is the caller's to set - this
	 * widget never sets its own, so the indicator has exactly one owner, which
	 * is AAlchemySimulatorPlayerController::UpdateInteractionIndicator.
	 */
	UFUNCTION(BlueprintCallable, Category = "InteractionIndicator")
	bool RepositionIndicator(const FVector NewPosition, float Distance, float MaxDistance);

protected:

	virtual void NativeConstruct() override;
};
