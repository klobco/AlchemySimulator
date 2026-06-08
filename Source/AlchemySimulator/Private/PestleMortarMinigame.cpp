// Fill out your copyright notice in the Description page of Project Settings.


#include "PestleMortarMinigame.h"

#include "CrushTargetWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UPestleMortarMinigame::NativeConstruct()
{
    Super::NativeConstruct();

    StartMinigame();
}

void UPestleMortarMinigame::NativeDestruct()
{
    Super::NativeDestruct();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SpawnTimerHandle);
    }
}

void UPestleMortarMinigame::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (bMinigameEnded)
    {
        return;
    }

    if (bPendingFirstSpawn && TargetCanvas)
    {
        const FVector2D CanvasSize = TargetCanvas->GetCachedGeometry().GetLocalSize();
        if (CanvasSize.X > 0.0 && CanvasSize.Y > 0.0)
        {
            bPendingFirstSpawn = false;
            SpawnCrushTarget();
        }
    }

    ElapsedTime += InDeltaTime;

    if (ElapsedTime >= MinigameDuration)
    {
        EndMinigame(CurrentGroundness >= RequiredGroundness);
    }
}

void UPestleMortarMinigame::StartMinigame()
{
    ElapsedTime = 0.f;
    CurrentGroundness = 0.f;
    Score = 0;
    PerfectHits = 0;
    Misses = 0;
    bMinigameEnded = false;

    UE_LOG(LogTemp, Warning, TEXT("Pestle mortar minigame started"));

    RefreshUI();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            SpawnTimerHandle,
            this,
            &UPestleMortarMinigame::SpawnCrushTarget,
            SpawnInterval,
            true
        );
    }

    bPendingFirstSpawn = true;
}

void UPestleMortarMinigame::SpawnCrushTarget()
{
    if (!TargetCanvas || !CrushTargetWidgetClass)
    {
        return;
    }

    UCrushTargetWidget* Target = CreateWidget<UCrushTargetWidget>(
        GetOwningPlayer(),
        CrushTargetWidgetClass
    );

    if (!Target)
    {
        return;
    }

    Target->OnTargetFinished.AddDynamic(this, &UPestleMortarMinigame::HandleTargetFinished);

    UCanvasPanelSlot* CanvasSlot = TargetCanvas->AddChildToCanvas(Target);
    if (!CanvasSlot)
    {
        return;
    }

    CanvasSlot->SetSize(TargetSize);
    CanvasSlot->SetPosition(GetRandomTargetPosition());
    CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));

    Target->InitTarget(TargetLifetime);
}

FVector2D UPestleMortarMinigame::GetRandomTargetPosition() const
{
    if (!TargetCanvas)
    {
        return FVector2D::ZeroVector;
    }

    const FGeometry CanvasGeometry = TargetCanvas->GetCachedGeometry();
    const FVector2D CanvasSize = CanvasGeometry.GetLocalSize();

    const float HalfWidth = TargetSize.X * 0.5f;
    const float HalfHeight = TargetSize.Y * 0.5f;

    const float X = FMath::RandRange(HalfWidth, static_cast<float>(CanvasSize.X) - HalfWidth);
    const float Y = FMath::RandRange(HalfHeight, static_cast<float>(CanvasSize.Y) - HalfHeight);

    return FVector2D(X, Y);
}

void UPestleMortarMinigame::HandleTargetFinished(ECrushHitResult HitResult)
{
    switch (HitResult)
    {
    case ECrushHitResult::Perfect:
        Score += 100;
        PerfectHits++;
        AddGroundness(20.f);
        break;

    case ECrushHitResult::Good:
        Score += 50;
        AddGroundness(12.f);
        break;

    case ECrushHitResult::Bad:
        Score += 10;
        AddGroundness(4.f);
        break;

    case ECrushHitResult::Miss:
        Score -= 10;
        Misses++;
        break;

    default:
        break;
    }

    RefreshUI();

    if (CurrentGroundness >= RequiredGroundness)
    {
        EndMinigame(true);
    }
}

void UPestleMortarMinigame::AddGroundness(float Amount)
{
    CurrentGroundness = FMath::Clamp(CurrentGroundness + Amount, 0.f, RequiredGroundness);
}

void UPestleMortarMinigame::RefreshUI()
{
    if (GroundnessBar)
    {
        GroundnessBar->SetPercent(CurrentGroundness / RequiredGroundness);
    }

    if (ScoreText)
    {
        ScoreText->SetText(FText::FromString(FString::Printf(TEXT("Score: %d"), Score)));
    }
}

void UPestleMortarMinigame::EndMinigame(bool bSuccess)
{
    if (bMinigameEnded)
    {
        return;
    }
    bMinigameEnded = true;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(SpawnTimerHandle);
    }

    FMinigameResult Result;
    Result.bSuccess = bSuccess;
    Result.QualityMultiplier = FMath::Clamp(
        float(PerfectHits) / float(FMath::Max(1, PerfectHits + Misses)),
        0.f, 1.f
    );

    UE_LOG(LogTemp, Warning, TEXT("Pestle mortar ended. Success: %s, Groundness: %.1f, Perfects: %d, Misses: %d, QM: %.2f"),
        bSuccess ? TEXT("true") : TEXT("false"),
        CurrentGroundness, PerfectHits, Misses, Result.QualityMultiplier
    );

    OnPestleMortarFinished.Broadcast(Result);
    FinishMinigame(Result);

    RemoveFromParent();
}
