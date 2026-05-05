// Fill out your copyright notice in the Description page of Project Settings.


#include "AlchemyMinigameWidget.h"

void UAlchemyMinigameWidget::FinishMinigame(bool bSuccess)
{
    OnMinigameFinished.Broadcast(bSuccess);
}