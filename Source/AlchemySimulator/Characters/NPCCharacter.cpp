// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NPCCharacter.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Controllers/AlchemyNPCConroller.h"

// Sets default values
ANPCCharacter::ANPCCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

    bUseControllerRotationYaw = false;

    GetCharacterMovement()->bOrientRotationToMovement = true;

    AIControllerClass = AAlchemyNPCConroller::StaticClass();
}

void ANPCCharacter::BeginPlay()
{
    Super::BeginPlay();
}