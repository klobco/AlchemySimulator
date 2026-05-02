// Fill out your copyright notice in the Description page of Project Settings.


#include "AlchemySimulatorCharacter.h"
#include "InventoryComponent.h"
#include "PlantPart.h"

// Sets default values
APlantPart::APlantPart()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlantPart::BeginPlay()
{
	Super::BeginPlay();
	
}

void APlantPart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FText APlantPart::GetInteractPrompt_Implementation() const
{
	return FText::GetEmpty();
}

FVector APlantPart::GetInteractWorldLocation_Implementation() const
{
	return GetActorLocation();
}

bool APlantPart::CanInteract_Implementation(APawn* By) const
{
	return true;
}

void APlantPart::Interact_Implementation(APawn* By)
{
	AAlchemySimulatorCharacter* player = Cast<AAlchemySimulatorCharacter>(By);
	//UE_LOG(LogTemp, Error, TEXT("Inventory owner: %s"), *player->inventory->GetOwner()->GetName());
	if (player && player->inventory != nullptr)
	{
		if (player->inventory->AddItem(Item, 1, Instance)) {
			Destroy();
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Interact with plant"));
}

void APlantPart::OnFocStart_Implementation(APawn* By)
{
	UE_LOG(LogTemp, Error, TEXT("Focus startorino"));
}

void APlantPart::OnFocEnd_Implementation(APawn* By)
{
	UE_LOG(LogTemp, Error, TEXT("Focus endorino"));
}

