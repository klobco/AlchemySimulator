// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractionCameraRig.generated.h"

UCLASS()
class ALCHEMYSIMULATOR_API AInteractionCameraRig : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractionCameraRig();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "Camera Tilt")
	float MaxTiltPitch = 8.0f;

	UPROPERTY(EditAnywhere, Category = "Camera Tilt")
	float MaxTiltYaw = 12.0f;

	UPROPERTY(EditAnywhere, Category = "Camera Tilt")
	float TiltInterpSpeed = 4.0f;

	bool bTiltActive = false;

	void EnableTilt();
	void DisableTilt();

private:
	FRotator BaseArmRotation = FRotator::ZeroRotator;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
