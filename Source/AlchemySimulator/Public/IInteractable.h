// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IInteractable.generated.h"

UINTERFACE(BlueprintType)
class ALCHEMYSIMULATOR_API UInteractable : public UInterface
{
	GENERATED_BODY()

};

/**
 * 
 */
class ALCHEMYSIMULATOR_API IInteractable
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FText GetInteractPrompt() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetInteractWorldLocation() const;


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool CanInteract(APawn* By) const;


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Interact(APawn* By);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnFocStart(APawn* By);
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnFocEnd(APawn* By);
};
