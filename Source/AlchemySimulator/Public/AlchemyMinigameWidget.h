// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AlchemyMinigameWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMinigameFinished, bool, bSuccess);
/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UAlchemyMinigameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

    UPROPERTY(BlueprintAssignable)
    FOnMinigameFinished OnMinigameFinished;

    UFUNCTION(BlueprintCallable)
    void FinishMinigame(bool bSuccess);
};
