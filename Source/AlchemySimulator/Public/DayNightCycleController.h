// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameTimeSubsystem.h"
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
	void UpdateSunRotation(const FAlchemyGameTime& NewTime);

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	class ADirectionalLight* SunLight;

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	float SunPitchOffset;

	UPROPERTY(EditAnywhere, Category = "DayNightCycle")
	class UGameTimeSubsystem* TimeSubsystem;

};
