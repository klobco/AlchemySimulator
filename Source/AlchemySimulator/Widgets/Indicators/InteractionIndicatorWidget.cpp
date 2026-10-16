// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionIndicatorWidget.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"

void UInteractionIndicatorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Constant for the widget's whole life, so it has no business in the
	// per-frame reposition path.
	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
}

bool UInteractionIndicatorWidget::RepositionIndicator(const FVector NewPosition, float Distance, float MaxDistance)
{
	if (!OuterCircle)
	{
		return false;
	}

	FVector2D ScreenPosition;

	// ProjectWorldLocationToWidgetPosition already divides the DPI scale out
	// (via ScreenToViewport), which is why SetPositionInViewport below must be
	// told not to remove it a second time.
	const bool bProjected = UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		GetOwningPlayer(),
		NewPosition,
		ScreenPosition,
		false
	);

	if (!bProjected)
	{
		return false;
	}

	// Deliberately grows with distance: a far target needs a bigger ring to stay
	// readable. This is not an inverted perspective scale - do not "fix" it.
	const float Scale = FMath::GetMappedRangeValueClamped(
		FVector2D(0.f, MaxDistance),
		FVector2D(0.8f, 2.0f),
		Distance
	);

	OuterCircle->SetRenderScale(FVector2D(Scale, Scale));
	SetPositionInViewport(ScreenPosition + Offset, false);

	return true;
}
