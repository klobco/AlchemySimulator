// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Widgets/Minigames/AlchemyMinigameWidget.h"
#include "MinigameManagerComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ALCHEMYSIMULATOR_API UMinigameManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMinigameManagerComponent();

protected:
	virtual void BeginPlay() override;

public:
    UPROPERTY(BlueprintAssignable)
    FOnMinigameFinished OnMinigameFinished;

    UFUNCTION(BlueprintCallable)
    void StartMinigame(TSubclassOf<UAlchemyMinigameWidget> MinigameWidgetClass);

	UFUNCTION(BlueprintCallable)
    void StopMinigame();

    /** The minigame currently on screen, or null. Drives input-mode priority. */
    UFUNCTION(BlueprintPure)
    UAlchemyMinigameWidget* GetActiveMinigameWidget() const { return ActiveMinigameWidget; }

private:
    UPROPERTY()
    TObjectPtr<UAlchemyMinigameWidget> ActiveMinigameWidget;

    UFUNCTION()
    void HandleMinigameFinished(bool bSuccess);

};
