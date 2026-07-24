// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Widgets/Menu/BaseGameWidget.h"
#include "CharacterScreenWidget.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UCharacterScreenWidget : public UBaseGameWidget
{
	GENERATED_BODY()
	

	UPROPERTY(meta = (BindWidget))
	class UInventoryWidget* PlayerInventoryWidget;

	UFUNCTION()
	void Setup();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void OnClosed_Implementation() override;

};
