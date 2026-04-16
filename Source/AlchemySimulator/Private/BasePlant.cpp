// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlant.h"
#include "AlchemySimulatorCharacter.h"
#include "AlchemySimulatorPlayerController.h"
#include "InventoryComponent.h"
#include "BaseTool.h"
#include "ItemDefinitionBase.h"
#include "BasicWorkbench.h"
#include "Components/BoxComponent.h"
#include "ItemDefinitionBase.h"

// Sets default values
ABasePlant::ABasePlant()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Stem = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Stem"));
	RootComponent = Stem;

	Leaf_A = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Leaf A"));
	Leaf_A->SetupAttachment(Stem);

	Leaf_B = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Leaf B"));
	Leaf_B->SetupAttachment(Stem);

	Fruit = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fruit"));
	Fruit->SetupAttachment(Stem);

}

// Called when the game starts or when spawned
void ABasePlant::BeginPlay()
{
	Super::BeginPlay();

	TArray<UStaticMeshComponent*> MeshComponents;
	this->GetComponents<UStaticMeshComponent>(MeshComponents);

	for (UStaticMeshComponent* comp : MeshComponents) {

		comp->OnBeginCursorOver.AddDynamic(this, &ABasePlant::HandleBeginCursorOver);
		comp->OnEndCursorOver.AddDynamic(this, &ABasePlant::HandleEndCursorOver);
		comp->OnClicked.AddDynamic(this, &ABasePlant::HandleClicked);
	}
	
}

void ABasePlant::SetPlantHighlight(bool bEnabled)
{

	if (bEnabled && HerbStatus == EHerbStatus::OnStand)
	{
		if (Stem)   Stem->SetOverlayMaterial(OverlayMaterialInstance);
		if (Leaf_A) Leaf_A->SetOverlayMaterial(OverlayMaterialInstance);
		if (Leaf_B) Leaf_B->SetOverlayMaterial(OverlayMaterialInstance);
		if (Fruit)  Fruit->SetOverlayMaterial(OverlayMaterialInstance);
	}
	else
	{
		if (Stem)   Stem->SetOverlayMaterial(nullptr);
		if (Leaf_A) Leaf_A->SetOverlayMaterial(nullptr);
		if (Leaf_B) Leaf_B->SetOverlayMaterial(nullptr);
		if (Fruit)  Fruit->SetOverlayMaterial(nullptr);
	}
}

void ABasePlant::HandleBeginCursorOver(UPrimitiveComponent* Component)
{
	SetPlantHighlight(true);
}

void ABasePlant::HandleEndCursorOver(UPrimitiveComponent* Component)
{
	SetPlantHighlight(false);
}

void ABasePlant::HandleClicked(UPrimitiveComponent* Component, FKey ButtonPressed)
{
	UE_LOG(LogTemp, Warning, TEXT("Plant clicked"));

	if (HerbStatus == EHerbStatus::OnStand && ParentWorkbench)
	{
		ParentWorkbench->MovePlantToManipulation(this);
	}

	if (HerbStatus == EHerbStatus::OnTable) {

		if (ParentWorkbench->ActiveToolIndex == INDEX_NONE) return;

		UE_LOG(
			LogTemp,
			Warning,
			TEXT("Clicked component: %s"),
			Component ? *Component->GetName() : TEXT("NULL")
		);

		ABaseTool* tool = *ParentWorkbench->Tools.Find(ParentWorkbench->ActiveToolIndex);


		if (tool->Item->Category == EItemCategory::Knife)
		{
			UE_LOG(LogTemp, Warning, TEXT("Not null index tools"));
			Component->SetVisibility(false,false);
		}
	}

	if (AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		// napr. ak chceš ma active plant namiesto active tool
		// PC->SetActivePlant(this);
	}
}

// Called every frame
void ABasePlant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FText ABasePlant::GetInteractPrompt_Implementation() const
{
	return FText::GetEmpty();
}

FVector ABasePlant::GetInteractWorldLocation_Implementation() const
{
	return GetActorLocation();
}

bool ABasePlant::CanInteract_Implementation(APawn* By) const
{
	return true;
}

void ABasePlant::Interact_Implementation(APawn* By)
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

void ABasePlant::OnFocStart_Implementation(APawn* By)
{
	UE_LOG(LogTemp, Error, TEXT("Focus startorino"));
}

void ABasePlant::OnFocEnd_Implementation(APawn* By)
{
	UE_LOG(LogTemp, Error, TEXT("Focus endorino"));
}
