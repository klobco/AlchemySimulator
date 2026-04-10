// Fill out your copyright notice in the Description page of Project Settings.


#include "InteractionCameraRig.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Sets default values
AInteractionCameraRig::AInteractionCameraRig()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraArm->SetupAttachment(RootComponent);
	CameraArm->TargetArmLength = 50.0f;

	// Create a follow camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	Camera->SetupAttachment(CameraArm, USpringArmComponent::SocketName);

}

// Called when the game starts or when spawned
void AInteractionCameraRig::BeginPlay()
{
	Super::BeginPlay();
}

void AInteractionCameraRig::EnableTilt()
{
	BaseArmRotation = CameraArm->GetRelativeRotation();
	bTiltActive = true;
}

void AInteractionCameraRig::DisableTilt()
{
	bTiltActive = false;
	CameraArm->SetRelativeRotation(BaseArmRotation);
}

// Called every frame
void AInteractionCameraRig::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bTiltActive) return;

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	int32 ViewportX, ViewportY;
	PC->GetViewportSize(ViewportX, ViewportY);
	if (ViewportX == 0 || ViewportY == 0) return;

	float MouseX, MouseY;
	if (!PC->GetMousePosition(MouseX, MouseY)) return;

	// Normalize mouse to [-1, 1] where (0, 0) is the screen center
	const float NormX =  (MouseX / ViewportX) * 2.0f - 1.0f;
	const float NormY =  (MouseY / ViewportY) * 2.0f - 1.0f;

	// Positive NormY = mouse below center = tilt camera down (negative pitch)
	const FRotator TiltOffset(-NormY * MaxTiltPitch, NormX * MaxTiltYaw, 0.0f);
	const FRotator TargetRelative = BaseArmRotation + TiltOffset;

	const FRotator NewRelative = FMath::RInterpTo(
		CameraArm->GetRelativeRotation(), TargetRelative, DeltaTime, TiltInterpSpeed);

	CameraArm->SetRelativeRotation(NewRelative);
}

