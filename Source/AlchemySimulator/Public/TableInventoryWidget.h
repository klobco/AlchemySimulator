// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TableInventoryWidget.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UTableInventoryWidget : public UUserWidget
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
