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
#include "Widgets/Minigames/AlchemyCutMinigameWidget.h"
#include "Widgets/Minigames/PestleMortarMinigame.h"
#include "ItemDefinitions/PlantItemDefinition.h"

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

			if (tool->Item->ToolCategory == EToolCategory::Knife)
			{
				APlayerController* PC = GetWorld()->GetFirstPlayerController();
				if (!PC) return;

				AAlchemySimulatorPlayerController* MyPC = Cast<AAlchemySimulatorPlayerController>(PC);
				if (!MyPC) return;



				if (!MyPC->MinigameManager) return;

				InteractedPart = Cast<UStaticMeshComponent>(Component);
				if (!InteractedPart) return;

				if (InteractedPart == Stem){
					
					UE_LOG(LogTemp, Warning, TEXT("[BasePlant] Cannot cut stem while leafs are still visible"));
					return;
				}

				MyPC->MinigameManager->OnMinigameFinished.AddDynamic(this, &ABasePlant::OnCuttingFinished);
				MyPC->MinigameManager->StartMinigame(CuttingMinigameWidgetClass);

				return;
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

void ABasePlant::OnCuttingFinished(bool bSuccess)
{
	if (AAlchemySimulatorPlayerController* MyPC = Cast<AAlchemySimulatorPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		MyPC->MinigameManager->OnMinigameFinished.RemoveDynamic(this, &ABasePlant::OnCuttingFinished);
	}

	if (bSuccess)
	{
		
		UE_LOG(LogTemp, Warning, TEXT("[BasePlant] Cutting succeeded"));

		FItemInstanceData NewInstance = Instance;

		if (bSuccess)
		{
			NewInstance.Quality = FMath::Clamp(NewInstance.Quality - 5, 0, 100);
		}
		else
		{
			NewInstance.Quality = FMath::Clamp(NewInstance.Quality - 25, 0, 100);
		}

		NewInstance.Freshness = 1.0f;
		NewInstance.bIsProcessed = false;
		NewInstance.ProcessingQuality = 1.0f;
		NewInstance.ProcessingTags.Reset();

        if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
        {
            if (APawn* P = PC->GetPawn())
            {
                if (UInventoryComponent* Inv = P->FindComponentByClass<UInventoryComponent>())
                {
					const FString PartName = InteractedPart->GetName();

					
					if (PartName.Contains(TEXT("Stem")))
					{
						Inv->AddItem(StemItem, 1,NewInstance);
					}
					else if (PartName.Contains(TEXT("Leaf")))
					{
						Inv->AddItem(LeafItem, 1,NewInstance);
					}
					else if (PartName.Contains(TEXT("Fruit")))
					{
						UE_LOG(LogTemp, Warning, TEXT("[BasePlant] Adding fruit to inventory"));
						bool was = Inv->AddItem(FruitItem, 1,NewInstance);
						UE_LOG(LogTemp, Warning, TEXT("[BasePlant] Adding fruit to inventory result: %s"), was ? TEXT("true") : TEXT("false"));
					}
                }
            }
        }

		if (InteractedPart)
		{
			InteractedPart->SetVisibility(false);
			InteractedPart->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		TArray<UStaticMeshComponent*> MeshComponents;
		GetComponents<UStaticMeshComponent>(MeshComponents);
		int32 visibleParts = 0;
		for (UStaticMeshComponent* comp : MeshComponents) {
			if (comp->IsVisible()) visibleParts++;
		}

		if (visibleParts == 1)
		{
			GetWorld()->GetFirstPlayerController()->GetPawn()->FindComponentByClass<UInventoryComponent>()->AddItem(StemItem, 1,NewInstance);
			ParentWorkbench->RemoveHerbItem(this);
			UE_LOG(LogTemp, Warning, TEXT("[BasePlant] All parts cut, destroying plant"));
			Destroy();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[BasePlant] Cutting failed"));
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
