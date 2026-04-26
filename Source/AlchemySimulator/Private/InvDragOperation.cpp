// Fill out your copyright notice in the Description page of Project Settings.

#include "InvDragOperation.h"
#include "AlchemySimulatorPlayerController.h"
#include "Engine/World.h"

void UInvDragOperation::DragCancelled_Implementation(const FPointerEvent& PointerEvent)
{
    Super::DragCancelled_Implementation(PointerEvent);

    UE_LOG(LogTemp, Warning, TEXT("DragCancelled on UInvDragOperation"));

    if (bDropHandledByUI)
    {
        UE_LOG(LogTemp, Warning, TEXT("Drag already handled by UI"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("No world in drag op"));
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("No player controller"));
        return;
    }

    AAlchemySimulatorPlayerController* MyPC = Cast<AAlchemySimulatorPlayerController>(PC);
    if (!MyPC)
    {
        UE_LOG(LogTemp, Error, TEXT("Wrong player controller type"));
        return;
    }

    const FVector2D ScreenPos = PointerEvent.GetScreenSpacePosition();

    const bool bHandled = MyPC->TryHandleWorldDropFromScreenPosition(this, ScreenPos);

    UE_LOG(LogTemp, Warning, TEXT("World drop handled: %s"), bHandled ? TEXT("true") : TEXT("false"));
}
