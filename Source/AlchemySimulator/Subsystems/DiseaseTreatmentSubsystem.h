// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DataStructHelpers.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DiseaseTreatmentSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDiseaseTreatmentSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:

FTreatmentResult EvaluateTreatment(
    const FPotionResult& Potion,
    const FPatientCondition& PatientCondition
);
};
