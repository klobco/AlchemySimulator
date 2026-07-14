// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Disease/PatientConditionComponent.h"
#include "Subsystems/DiseaseTreatmentSubsystem.h"

// Sets default values for this component's properties
UPatientConditionComponent::UPatientConditionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPatientConditionComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UPatientConditionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

FTreatmentResult UPatientConditionComponent::ApplyPotion(const FItemInstanceData& PotionInstance)
{
	if (!PotionInstance.bIsPotion)
	{
		UE_LOG(LogTemp, Warning, TEXT("Tried to apply a non-potion item as treatment!"));
		return FTreatmentResult();
	}
	

	FTreatmentResult Result = GetWorld()->GetGameInstance()->GetSubsystem<UDiseaseTreatmentSubsystem>()->EvaluateTreatment(PotionInstance.PotionResult, CurrentCondition);

	return Result;
}