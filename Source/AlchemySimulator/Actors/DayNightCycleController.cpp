// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/DayNightCycleController.h"
#include "Engine/DirectionalLight.h"

// Sets default values
ADayNightCycleController::ADayNightCycleController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;



}

// Called when the game starts or when spawned
void ADayNightCycleController::BeginPlay()
{
	Super::BeginPlay();
	
	TimeSubsystem = GetWorld()->GetSubsystem<UGameTimeSubsystem>();

	TimeSubsystem->OnGameMinuteChanged.AddDynamic(this, &ADayNightCycleController::UpdateSunRotation);
}

// Called every frame
void ADayNightCycleController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ADayNightCycleController::UpdateSunRotation(const FAlchemyGameTime& NewTime)
{
	if (!SunLight || !TimeSubsystem)
	{
		return;
	}

	const float DayProgress = TimeSubsystem->GetNormalizedDay();

	const float SunPitch = DayProgress * 360.0f + SunPitchOffset;

	FRotator NewRotation(SunPitch, SunYaw, 0.0f);

	SunLight->SetActorRotation(NewRotation);
}

