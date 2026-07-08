// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/DiseaseTreatmentSubsystem.h"

FTreatmentResult UDiseaseTreatmentSubsystem::EvaluateTreatment(
    const FPotionResult& Potion,
    const FPatientCondition& PatientCondition
)
{
    //TODO: Implement actual evaluation logic comparing the potion's effects against the patient's condition and the disease's treatment requirements
    FTreatmentResult Result;
    return Result;
}