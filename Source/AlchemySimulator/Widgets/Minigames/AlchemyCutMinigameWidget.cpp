// Fill out your copyright notice in the Description page of Project Settings.


#include "AlchemyCutMinigameWidget.h"

#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Border.h"
#include "TimerManager.h"
#include "InputCoreTypes.h"

void UAlchemyCutMinigameWidget::NativeConstruct()
{
    Super::NativeConstruct();

    SetIsFocusable(true);
    SetFocus();

    BuildSegments();
    StartMinigame();

    FSlateApplication::Get().SetKeyboardFocus(TakeWidget(), EFocusCause::SetDirectly);
}

void UAlchemyCutMinigameWidget::NativeDestruct()
{
    Super::NativeDestruct();

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MoveTimerHandle);
    }
}

void UAlchemyCutMinigameWidget::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnFocusLost(InFocusEvent);
    if (bIsRunning && !bFinished)
    {
        SetFocus();
    }
}

FReply UAlchemyCutMinigameWidget::NativeOnKeyDown(
    const FGeometry& InGeometry,
    const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::SpaceBar)
    {
        FinishCut();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UAlchemyCutMinigameWidget::BuildSegments()
{
    if (!SegmentContainer) return;

    SegmentContainer->ClearChildren();
    Segments.Empty();

    SegmentCount = FMath::Max(1, SegmentCount);
    GreenSegmentCount = FMath::Clamp(GreenSegmentCount, 1, SegmentCount);

    GreenStartIndex = FMath::RandRange(0, SegmentCount - GreenSegmentCount);
    GreenEndIndex = GreenStartIndex + GreenSegmentCount - 1;

    for (int32 i = 0; i < SegmentCount; i++)
    {
        UBorder* Segment = NewObject<UBorder>(this);
        if (!Segment) continue;

        Segment->SetBrushColor(RedColor);

        UHorizontalBoxSlot* SlotSegment = SegmentContainer->AddChildToHorizontalBox(Segment);
        if (SlotSegment)
        {
            SlotSegment->SetPadding(FMargin(2.f));
            SlotSegment->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        }

        Segments.Add(Segment);
    }

    RefreshVisuals();
}

void UAlchemyCutMinigameWidget::StartMinigame()
{
    CurrentIndex = 0;
    bIsRunning = true;
    bFinished = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(
            MoveTimerHandle,
            this,
            &UAlchemyCutMinigameWidget::MoveIndicator,
            MoveInterval,
            true
        );
    }

    RefreshVisuals();
}

void UAlchemyCutMinigameWidget::MoveIndicator()
{
    if (!bIsRunning || bFinished) return;

    CurrentIndex++;

    if (CurrentIndex >= SegmentCount)
    {
        CurrentIndex = 0;
    }

    RefreshVisuals();
}

void UAlchemyCutMinigameWidget::RefreshVisuals()
{
    for (int32 i = 0; i < Segments.Num(); i++)
    {
        if (!Segments[i]) continue;

        if (i == CurrentIndex)
        {
            Segments[i]->SetBrushColor(IndicatorColor);
        }
        else if (i >= GreenStartIndex && i <= GreenEndIndex)
        {
            Segments[i]->SetBrushColor(GreenColor);
        }
        else
        {
            Segments[i]->SetBrushColor(RedColor);
        }
    }
}

void UAlchemyCutMinigameWidget::FinishCut()
{
    if (bFinished) return;

    bFinished = true;
    bIsRunning = false;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(MoveTimerHandle);
    }

    FMinigameResult Result;

    const bool bSuccess = IsCurrentIndexSuccess();

    if (bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cutting minigame SUCCESS. Index: %d"), CurrentIndex);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Cutting minigame FAIL. Index: %d"), CurrentIndex);
    }

    Result.bSuccess = bSuccess;
    FinishMinigame(Result);
}

bool UAlchemyCutMinigameWidget::IsCurrentIndexSuccess() const
{
    return CurrentIndex >= GreenStartIndex && CurrentIndex <= GreenEndIndex;
}