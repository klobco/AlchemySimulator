// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Plants/PlantPart.h"
#include "AlchemySimulatorCharacter.h"
#include "AlchemySimulatorPlayerController.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Actors/Stations/BasicWorkbench.h"
#include "Actors/Tools/BaseTool.h"
#include "DataAssets/DataAssetProcessingMethod.h"
#include "ItemDefinitions/ToolItemDefinition.h"
#include "Components/Minigame/MinigameManagerComponent.h"
#include "Components/Processing/ProcessingComponent.h"

// Sets default values
APlantPart::APlantPart()
{
 	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	RootComponent = Body;
}

// Called when the game starts or when spawned
void APlantPart::BeginPlay()
{
	Super::BeginPlay();

	// BP_PlantPart may have been saved while bCanEverTick was false; a serialized
	// Blueprint value overrides the C++ constructor default, so force it on here.
	PrimaryActorTick.bCanEverTick = true;
	if (!PrimaryActorTick.IsTickFunctionRegistered())
	{
		PrimaryActorTick.RegisterTickFunction(GetLevel());
	}
	PrimaryActorTick.SetTickFunctionEnable(true);

	if (Body)
	{
		Body->OnBeginCursorOver.AddDynamic(this, &APlantPart::HandleBeginCursorOver);
		Body->OnEndCursorOver.AddDynamic(this, &APlantPart::HandleEndCursorOver);
		Body->OnClicked.AddDynamic(this, &APlantPart::HandleClicked);
	}
}

void APlantPart::HandleBeginCursorOver(UPrimitiveComponent* Component)
{
	if (Body && OverlayMaterialInstance && Component == Body)
	{
		Body->SetOverlayMaterial(OverlayMaterialInstance);
	}
}

void APlantPart::HandleEndCursorOver(UPrimitiveComponent* Component)
{
	if (Body && Component == Body)
	{
		Body->SetOverlayMaterial(nullptr);
	}
}

void APlantPart::HandleClicked(UPrimitiveComponent* Component, FKey ButtonPressed)
{
	UE_LOG(LogTemp, Warning, TEXT("[PlantPart::HandleClicked] component: %s"), *GetNameSafe(Component));

	if (HerbStatus != EHerbStatus::OnTable) return;
	if (!ParentWorkbench) return;

	AAlchemySimulatorPlayerController* MyPC = Cast<AAlchemySimulatorPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!MyPC || !MyPC->MinigameManager) return;

	// Find() returns null for a stale ActiveToolIndex — never dereference it blind.
	ABaseTool* const* ToolPtr = ParentWorkbench->Tools.Find(ParentWorkbench->ActiveToolIndex);
	ABaseTool* Tool = ToolPtr ? *ToolPtr : nullptr;

	// No tool held, or this tool does nothing to a plant part: fall back to dragging it.
	if (!Tool || !Tool->Item || !MyPC->MinigameManager->TryStartToolAction(Tool->Item, this, Component))
	{
		MyPC->StartWorldDrag(this);
	}
}

bool APlantPart::CanAcceptToolAction_Implementation(UToolItemDefinition* Tool, const FToolAction& Action, UPrimitiveComponent* HitComponent) const
{
	if (HerbStatus != EHerbStatus::OnTable) return false;
	if (!AcceptedToolActions.HasTag(Action.ActionTag)) return false;

	// Processing actions are additionally gated by the plant part's own data.
	if (Action.ProcessingMethod)
	{
		if (!Item || !Item->bCanBeProcessed) return false;
		if (!Item->AllowedProcessingTags.HasTag(Action.ProcessingMethod->ProcessingTag)) return false;
	}

	return true;
}

void APlantPart::ApplyToolActionResult_Implementation(UToolItemDefinition* Tool, const FToolAction& Action, const FMinigameResult& Result, UPrimitiveComponent* HitComponent)
{
	if (!Result.bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[PlantPart] Action %s failed"), *Action.ActionTag.ToString());
		return;
	}

	// The minigame's QualityMultiplier is what the old hardcoded 1.15f bonus stood in for.
	const FItemInstanceData NewInstance = UProcessingComponent::BuildProcessedInstance(
		Instance, Action.ProcessingMethod, Result.QualityMultiplier);

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* P = PC->GetPawn())
		{
			if (UInventoryComponent* Inv = P->FindComponentByClass<UInventoryComponent>())
			{
				Inv->AddItem(Item, 1, NewInstance);
			}
		}
	}

	if (ParentWorkbench)
	{
		ParentWorkbench->RemoveHerbItem(this);
	}
	Destroy();
}

void APlantPart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HerbStatus == EHerbStatus::OnTable && ParentWorkbench && Body && Body->IsSimulatingPhysics())
	{
		const FVector MyLoc = GetActorLocation();
		const FVector Clamped = ParentWorkbench->ClampActorToWorkbench(this, MyLoc);
		if (!MyLoc.Equals(Clamped, 0.5f))
		{
			SetActorLocation(Clamped, false, nullptr, ETeleportType::TeleportPhysics);
			Body->SetPhysicsLinearVelocity(FVector::ZeroVector);
		}
	}
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
	if (player && player->inventory != nullptr)
	{
		if (player->inventory->AddItem(Item, 1, Instance)) {
			Destroy();
		}
	}
	UE_LOG(LogTemp, Error, TEXT("Interact with plant part"));
}

void APlantPart::OnFocStart_Implementation(APawn* By)
{
	UE_LOG(LogTemp, Error, TEXT("Focus startorino"));
}

void APlantPart::OnFocEnd_Implementation(APawn* By)
{
	UE_LOG(LogTemp, Error, TEXT("Focus endorino"));
}
