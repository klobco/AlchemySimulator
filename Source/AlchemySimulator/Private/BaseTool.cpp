// Fill out your copyright notice in the Description page of Project Settings.

#include "AlchemySimulatorCharacter.h"
#include "InventoryComponent.h"
#include "ToolItemDefinition.h"
#include "BaseTool.h"
#include "AlchemySimulatorPlayerController.h"

// Sets default values
ABaseTool::ABaseTool()
{
	PrimaryActorTick.bCanEverTick = false;

	ToolMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ToolMesh"));
	RootComponent = ToolMesh;

}

void ABaseTool::BeginPlay()
{
	Super::BeginPlay();

	TArray<UStaticMeshComponent*> Meshes;
	GetComponents<UStaticMeshComponent>(Meshes);

	for (UStaticMeshComponent* Mesh : Meshes)
	{
		if (!Mesh) continue;

		Mesh->OnBeginCursorOver.AddDynamic(this, &ABaseTool::HandleBeginCursorOver);
		Mesh->OnEndCursorOver.AddDynamic(this, &ABaseTool::HandleEndCursorOver);
		Mesh->OnClicked.AddDynamic(this, &ABaseTool::HandleClicked);
	}
	
}

void ABaseTool::HandleBeginCursorOver(UPrimitiveComponent* Component)
{
	if (OverlayMaterialInstance)
	{
		TArray<UStaticMeshComponent*> Meshes;
		GetComponents<UStaticMeshComponent>(Meshes);

		for (UStaticMeshComponent* Mesh : Meshes)
		{
			if (!Mesh) continue;

			Mesh->SetOverlayMaterial(OverlayMaterialInstance);
		}
	}
}

void ABaseTool::HandleEndCursorOver(UPrimitiveComponent* Component)
{
	if (OverlayMaterialInstance)
	{
		TArray<UStaticMeshComponent*> Meshes;
		GetComponents<UStaticMeshComponent>(Meshes);

		for (UStaticMeshComponent* Mesh : Meshes)
		{
			if (!Mesh) continue;

			Mesh->SetOverlayMaterial(nullptr);
		}
	}
}

void ABaseTool::HandleClicked(UPrimitiveComponent* Component, FKey ButtonPressed) {
	UE_LOG(LogTemp, Error, TEXT("Clicked"));

	if (AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->SetActiveTool(this);
	}
}

FText ABaseTool::GetInteractPrompt_Implementation() const
{
	return FText::GetEmpty();
}

FVector ABaseTool::GetInteractWorldLocation_Implementation() const
{
	return GetActorLocation();
}

bool ABaseTool::CanInteract_Implementation(APawn* By) const
{
	return true;
}

void ABaseTool::Interact_Implementation(APawn* By)
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

void ABaseTool::OnFocStart_Implementation(APawn* By)
{
	UE_LOG(LogTemp, Error, TEXT("Focus startorino"));
}

void ABaseTool::OnFocEnd_Implementation(APawn* By)
{
	UE_LOG(LogTemp, Error, TEXT("Focus endorino"));
}
