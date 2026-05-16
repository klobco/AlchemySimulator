// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseTool.h"
#include "ToolPestleAndMortar.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API AToolPestleAndMortar : public ABaseTool
{
	GENERATED_BODY()
	

public:
	AToolPestleAndMortar();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* MortarMesh;


};
