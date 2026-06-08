// Fill out your copyright notice in the Description page of Project Settings.


#include "AlchemyMinigameWidget.h"

void UAlchemyMinigameWidget::FinishMinigame(FMinigameResult Result)
{
    OnMinigameFinished.Broadcast(Result.bSuccess);
    OnMinigameResultFinished.Broadcast(Result);
}