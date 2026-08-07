// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Plants/BasePlant.h"
#include "AlchemySimulatorCharacter.h"
#include "AlchemySimulatorPlayerController.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Actors/Tools/BaseTool.h"
#include "ItemDefinitions/PlantItemDefinition.h"
#include "DrawDebugHelpers.h"
#include "Actors/Stations/BasicWorkbench.h"
#include "ItemMetadata.h"
#include "Components/BoxComponent.h"
#include "Components/Minigame/MinigameManagerComponent.h"
#include "ItemDefinitions/ToolItemDefinition.h"

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

		if (!comp) continue;
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

	if (HerbStatus != EHerbStatus::OnTable) return;
	if (!ParentWorkbench) return;

	AAlchemySimulatorPlayerController* MyPC = Cast<AAlchemySimulatorPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!MyPC || !MyPC->MinigameManager) return;

	// Find() returns null for a stale ActiveToolIndex — never dereference it blind.
	ABaseTool* const* ToolPtr = ParentWorkbench->Tools.Find(ParentWorkbench->ActiveToolIndex);
	ABaseTool* Tool = ToolPtr ? *ToolPtr : nullptr;

	// No tool held, or this tool does nothing to this plant: fall back to dragging it.
	if (!Tool || !Tool->Item || !MyPC->MinigameManager->TryStartToolAction(Tool->Item, this, Component))
	{
		MyPC->StartWorldDrag(this);
	}
}

// Called every frame
void ABasePlant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HerbStatus == EHerbStatus::OnTable && ParentWorkbench && Stem && Stem->IsSimulatingPhysics())
	{
		const FVector MyLoc = GetActorLocation();
		const FVector Clamped = ParentWorkbench->ClampActorToWorkbench(this, MyLoc);
		if (!MyLoc.Equals(Clamped, 0.5f))
		{
			SetActorLocation(Clamped, false, nullptr, ETeleportType::TeleportPhysics);
			Stem->SetPhysicsLinearVelocity(FVector::ZeroVector);
		}
	}
}

bool ABasePlant::CanAcceptToolAction_Implementation(UToolItemDefinition* Tool, const FToolAction& Action, UPrimitiveComponent* HitComponent) const
{
	if (HerbStatus != EHerbStatus::OnTable) return false;
	if (!AcceptedToolActions.HasTag(Action.ActionTag)) return false;

	// Only the individual parts can be cut — the stem is what's left once they're gone.
	const UStaticMeshComponent* Part = Cast<UStaticMeshComponent>(HitComponent);
	if (!Part || Part == Stem)
	{
		return false;
	}

	return true;
}

void ABasePlant::ApplyToolActionResult_Implementation(UToolItemDefinition* Tool, const FToolAction& Action, const FMinigameResult& Result, UPrimitiveComponent* HitComponent)
{
	if (!Result.bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BasePlant] Cutting failed"));
		return;
	}

	UStaticMeshComponent* CutPart = Cast<UStaticMeshComponent>(HitComponent);
	if (!CutPart) return;

	UE_LOG(LogTemp, Warning, TEXT("[BasePlant] Cutting succeeded"));

	// Cutting harvests a raw part — it deliberately clears any processing state.
	FItemInstanceData NewInstance = Instance;
	NewInstance.Quality = FMath::Clamp(NewInstance.Quality - 5, 0, 100);
	NewInstance.Freshness = 1.0f;
	NewInstance.bIsProcessed = false;
	NewInstance.ProcessingQuality = 1.0f;
	NewInstance.ProcessingTags.Reset();

	UInventoryComponent* Inv = nullptr;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* P = PC->GetPawn())
		{
			Inv = P->FindComponentByClass<UInventoryComponent>();
		}
	}

	if (Inv)
	{
		// TODO: name matching is fragile — a component-to-UDataAssetPlantPart map would be better.
		const FString PartName = CutPart->GetName();

		if (PartName.Contains(TEXT("Stem")))
		{
			Inv->AddItem(StemItem, 1, NewInstance);
		}
		else if (PartName.Contains(TEXT("Leaf")))
		{
			Inv->AddItem(LeafItem, 1, NewInstance);
		}
		else if (PartName.Contains(TEXT("Fruit")))
		{
			UE_LOG(LogTemp, Warning, TEXT("[BasePlant] Adding fruit to inventory"));
			bool was = Inv->AddItem(FruitItem, 1, NewInstance);
			UE_LOG(LogTemp, Warning, TEXT("[BasePlant] Adding fruit to inventory result: %s"), was ? TEXT("true") : TEXT("false"));
		}
	}

	CutPart->SetVisibility(false);
	CutPart->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TArray<UStaticMeshComponent*> MeshComponents;
	GetComponents<UStaticMeshComponent>(MeshComponents);
	int32 visibleParts = 0;
	for (UStaticMeshComponent* comp : MeshComponents) {
		if (comp->IsVisible()) visibleParts++;
	}

	if (visibleParts == 1)
	{
		if (Inv)
		{
			Inv->AddItem(StemItem, 1, NewInstance);
		}
		if (ParentWorkbench)
		{
			ParentWorkbench->RemoveHerbItem(this);
		}
		UE_LOG(LogTemp, Warning, TEXT("[BasePlant] All parts cut, destroying plant"));
		Destroy();
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
