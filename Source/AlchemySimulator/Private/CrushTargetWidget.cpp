// Fill out your copyright notice in the Description page of Project Settings.


#include "CrushTargetWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"

void UCrushTargetWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ButtonHit)
    {
        ButtonHit->OnClicked.AddDynamic(this, &UCrushTargetWidget::HandleHitClicked);
    }
}

void UCrushTargetWidget::InitTarget(float InLifetime)
{
    Lifetime = FMath::Max(0.01f, InLifetime);
    ElapsedTime = 0.f;
    bIsActive = true;

    UpdateVisuals(0.f);
}

void UCrushTargetWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!bIsActive)
    {
        return;
    }

    ElapsedTime += InDeltaTime;

    const float Progress = FMath::Clamp(ElapsedTime / Lifetime, 0.f, 1.f);

    UpdateVisuals(Progress);

    if (Progress >= 1.f)
    {
        ExpireTarget();
    }
}

void UCrushTargetWidget::HandleHitClicked()
{
    if (!bIsActive)
    {
        return;
    }

    EvaluateHit();
}

void UCrushTargetWidget::UpdateVisuals(float Progress)
{
    if (!ImageOuterCircle)
    {
        return;
    }

    const float CurrentScale = FMath::Lerp(StartOuterScale, EndOuterScale, Progress);

    ImageOuterCircle->SetRenderScale(FVector2D(CurrentScale, CurrentScale));

    const float Opacity = FMath::Lerp(1.f, 0.25f, Progress);
    ImageOuterCircle->SetRenderOpacity(Opacity);
}

void UCrushTargetWidget::EvaluateHit()
{
    bIsActive = false;

    const float Progress = FMath::Clamp(ElapsedTime / Lifetime, 0.f, 1.f);
    const float DistanceFromPerfect = FMath::Abs(Progress - PerfectProgress);

    ECrushHitResult HitResult;
    if (DistanceFromPerfect <= PerfectTolerance)
    {
        HitResult = ECrushHitResult::Perfect;
    }
    else if (DistanceFromPerfect <= GoodTolerance)
    {
        HitResult = ECrushHitResult::Good;
    }
    else
    {
        HitResult = ECrushHitResult::Bad;
    }

    OnTargetFinished.Broadcast(HitResult);
    RemoveFromParent();
}

void UCrushTargetWidget::ExpireTarget()
{
    bIsActive = false;

    OnTargetFinished.Broadcast(ECrushHitResult::Miss);
    RemoveFromParent();
}
