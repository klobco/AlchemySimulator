// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AIController.h"
#include "AlchemyNPCConroller.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API AAlchemyNPCConroller : public AAIController
{
	GENERATED_BODY()

public:
    AAlchemyNPCConroller();

    UFUNCTION(BlueprintCallable, Category="AI")
    void SendBrainEvent(FGameplayTag EventTag);
	
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
    TObjectPtr<class UStateTreeAIComponent> StateTreeAIComponent;
};
