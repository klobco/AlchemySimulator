// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameTimeSubsystem.h"
#include "DayNightCycleController.generated.h"

UCLASS()
class ALCHEMYSIMULATOR_API ADayNightCycleController : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADayNightCycleController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	UFUNCTION()
	void UpdateCelestialRotation(const FAlchemyGameTime& NewTime);

	float CalculateSunAngleDegrees(const FAlchemyGameTime& Time) const;

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	class ADirectionalLight* SunLight;

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	float SunPitchOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	float SunYaw = 0.0f;

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	class ADirectionalLight* MoonLight;

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	float MoonPitchOffset = 0.0f;

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	float MoonYaw = 0.0f;

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	float SunriseHour = 4.0f;

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	float SunsetHour = 20.0f;

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	class UGameTimeSubsystem* TimeSubsystem;

};
