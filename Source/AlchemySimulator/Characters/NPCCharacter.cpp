// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/NPCCharacter.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Components/Disease/PatientConditionComponent.h"
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

    InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    PatientConditionComponent = CreateDefaultSubobject<UPatientConditionComponent>(TEXT("PatientConditionComponent"));
}

void ANPCCharacter::BeginPlay()
{
    Super::BeginPlay();
}


FText ANPCCharacter::GetInteractPrompt_Implementation() const
{
    return FText::Format(NSLOCTEXT("NPC","TalkTo","Talk to {0}"), DisplayName);
}

FVector ANPCCharacter::GetInteractWorldLocation_Implementation() const
{
    return GetActorLocation() + FVector(0,0,60.f);
}

bool ANPCCharacter::CanInteract_Implementation(APawn* By) const
{
    // Busy means "talking to somebody else" - whoever already holds the
    // conversation may keep interacting, or they could never end it.
    return CurrentInteractor == nullptr || CurrentInteractor == By;
}

void ANPCCharacter::Interact_Implementation(APawn* By)
{
    CurrentInteractor = By;
    UE_LOG(LogTemp, Warning, TEXT("%s: Hi, I'm %s"), *GetName(), *DisplayName.ToString());
}

void ANPCCharacter::EndInteraction(APawn* By)
{
    // Only the pawn holding the conversation may release it.
    if (CurrentInteractor == By)
    {
        CurrentInteractor = nullptr;
    }
}

void ANPCCharacter::OnFocStart_Implementation(APawn* By) {}

void ANPCCharacter::OnFocEnd_Implementation(APawn* By)
{
    // Looking away ends the conversation. Once a dialogue widget exists this
    // release belongs on its close instead, or walking away mid-line frees the NPC.
    EndInteraction(By);
}