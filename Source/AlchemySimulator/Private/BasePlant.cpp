// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlant.h"
#include "AlchemySimulatorCharacter.h"
#include "AlchemySimulatorPlayerController.h"
#include "InventoryComponent.h"
#include "BaseTool.h"
#include "ItemDefinitionBase.h"
#include "DrawDebugHelpers.h"
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

	UE_LOG(LogTemp, Warning, TEXT("base plant is in begin play"));

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
	UE_LOG(LogTemp, Warning, TEXT("[HandleClicked] component: %s"), *GetNameSafe(Component));

	if (HerbStatus == EHerbStatus::OnTable) {
		
		if (ParentWorkbench->ActiveToolIndex == INDEX_NONE)		 {
			APlayerController* PC = GetWorld()->GetFirstPlayerController();
			if (!PC) return;

			AAlchemySimulatorPlayerController* MyPC = Cast<AAlchemySimulatorPlayerController>(PC);
			if (!MyPC) return;

			MyPC->StartWorldDrag(this);
			return;
		}
		else
		{
			ABaseTool* tool = *ParentWorkbench->Tools.Find(ParentWorkbench->ActiveToolIndex);

			if (tool->Item->Category == EItemCategory::Knife)
			{
				UE_LOG(LogTemp, Warning, TEXT("Not null index tools"));
				Component->SetVisibility(false,false);
			}
		}


		

	}

	if (AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		// napr. ak chce� ma� active plant namiesto active tool
		// PC->SetActivePlant(this);
	}
}

// Called every frame
void ABasePlant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HerbStatus == EHerbStatus::OnTable && ParentWorkbench && Stem && Stem->IsSimulatingPhysics())
	{
		const FVector MyLoc = GetActorLocation();
		const FVector Clamped = ParentWorkbench->ClampLocationToWorkbench(MyLoc);
		if (!MyLoc.Equals(Clamped, 0.5f))
		{
			SetActorLocation(Clamped, false, nullptr, ETeleportType::TeleportPhysics);
			Stem->SetPhysicsLinearVelocity(FVector::ZeroVector);
		}
	}
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
