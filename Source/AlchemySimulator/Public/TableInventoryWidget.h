// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameWidget.h"
#include "TableInventoryWidget.generated.h"

/**
 * Widget showing both the station inventory and the player inventory side by side.
 * Pushed onto the modal stack by ABasicInteractableStationObject.
 */
UCLASS()
class ALCHEMYSIMULATOR_API UTableInventoryWidget : public UBaseGameWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	class UInventoryWidget* PlayerInventoryWidget;

	UPROPERTY(meta = (BindWidget))
	UInventoryWidget* TableInventoryWidget;


	UFUNCTION()
	void Setup(class ABasicInteractableStationObject* station);

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	
};
