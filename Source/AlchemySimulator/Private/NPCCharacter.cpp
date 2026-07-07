// Fill out your copyright notice in the Description page of Project Settings.


#include "NPCCharacter.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ANPCCharacter::ANPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    bUseControllerRotationYaw = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;
}

void ANPCCharacter::BeginPlay()
{
    Super::BeginPlay();


    AAIController* NPCController = Cast<AAIController>(GetController());
    NPCController->MoveToLocation(FVector(0.0f, 0.0f, 0.0f));
}