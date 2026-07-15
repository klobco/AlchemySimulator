// Fill out your copyright notice in the Description page of Project Settings.


#include "AlchemySimulatorCharacter.h"
#include "Components/Inventory/InventoryComponent.h"
#include "ItemDefinitions/PotionItemDefinition.h"
#include "Actors/Potions/BasePotion.h"

// Sets default values
ABasePotion::ABasePotion()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PotionMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PotionMesh"));
	RootComponent = PotionMesh;

	LiquidMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LiquidMesh"));
	LiquidMesh->SetupAttachment(RootComponent);


	
}

// Called when the game starts or when spawned
void ABasePotion::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABasePotion::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FText ABasePotion::GetInteractPrompt_Implementation() const
{
	return FText::GetEmpty();
}

FVector ABasePotion::GetInteractWorldLocation_Implementation() const
{
	return GetActorLocation();
}

bool ABasePotion::CanInteract_Implementation(APawn* By) const
{
	return true;
}

void ABasePotion::Interact_Implementation(APawn* By)
{
	AAlchemySimulatorCharacter* player = Cast<AAlchemySimulatorCharacter>(By);
	//UE_LOG(LogTemp, Error, TEXT("Inventory owner: %s"), *player->inventory->GetOwner()->GetName());
	if (player && player->inventory != nullptr)
	{
		if (player->inventory->AddItem(Item, 1, Instance)) {
			Destroy();
			UE_LOG(LogTemp, Error, TEXT("Adding tool : %s"), *GetNameSafe(Item));
		}
	}
}

void ABasePotion::OnFocStart_Implementation(APawn* By)
{
	UE_LOG(LogTemp, Error, TEXT("Focus startorino"));
}

void ABasePotion::OnFocEnd_Implementation(APawn* By)
{
	UE_LOG(LogTemp, Error, TEXT("Focus endorino"));
}
