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

	TimeSubsystem->OnGameMinuteChanged.AddDynamic(this, &ADayNightCycleController::UpdateCelestialRotation);
}

// Called every frame
void ADayNightCycleController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

float ADayNightCycleController::CalculateSunAngleDegrees(const FAlchemyGameTime& Time) const
{
	const float TimeOfDayHours = Time.Hour + Time.Minute / 60.0f;
	const float DayLength = SunsetHour - SunriseHour;
	const float NightLength = 24.0f - DayLength;

	if (TimeOfDayHours >= SunriseHour && TimeOfDayHours < SunsetHour)
	{
		const float DayFraction = (TimeOfDayHours - SunriseHour) / DayLength;
		return DayFraction * 180.0f;
	}

	float HoursSinceSunset = TimeOfDayHours - SunsetHour;
	if (HoursSinceSunset < 0.0f)
	{
		HoursSinceSunset += 24.0f;
	}
	const float NightFraction = HoursSinceSunset / NightLength;
	return 180.0f + NightFraction * 180.0f;
}

void ADayNightCycleController::UpdateCelestialRotation(const FAlchemyGameTime& NewTime)
{
	if (!SunLight || !TimeSubsystem)
	{
		return;
	}

	const float SunAngle = CalculateSunAngleDegrees(NewTime);

	// UE_LOG(LogTemp, Log, TEXT("Sun Angle: %f"), SunAngle);
	// UE_LOG(LogTemp, Log, TEXT("Time is: %s"), *NewTime.ToString());

	SunLight->SetActorRotation(FRotator(-SunAngle + SunPitchOffset, SunYaw, 0.0f));

	if (MoonLight)
	{
		const float MoonAngle = FMath::Fmod(SunAngle + 180.0f, 360.0f);
		MoonLight->SetActorRotation(FRotator(-MoonAngle + MoonPitchOffset, MoonYaw, 0.0f));
	}
}

