// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IUsebale.generated.h"

UINTERFACE(BlueprintType)
class ALCHEMYSIMULATOR_API UUsebale : public UInterface
{
	GENERATED_BODY()

};

/**
 * 
 */
class ALCHEMYSIMULATOR_API IUsebale
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void Use(APawn* By);
};