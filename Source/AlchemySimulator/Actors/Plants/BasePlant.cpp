// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Plants/BasePlant.h"
#include "AlchemySimulatorCharacter.h"
#include "AlchemySimulatorPlayerController.h"
#include "Components/Inventory/InventoryComponent.h"
#include "Actors/Tools/BaseTool.h"
#include "ItemDefinitions/PlantItemDefinition.h"
#include "DataAssets/DataAssetPlantPart.h"
#include "DrawDebugHelpers.h"
#include "Actors/Stations/BasicWorkbench.h"
#include "ItemMetadata.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/Minigame/MinigameManagerComponent.h"
#include "ItemDefinitions/ToolItemDefinition.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"

namespace
{
	/**
	 * Marks components built by RebuildPartComponents. Teardown scans for this tag rather than
	 * trusting GeneratedPartComponents, because that array is Transient and so does not survive
	 * the actor duplication that happens when a level-placed plant enters PIE — the components
	 * themselves do, and without the tag they would be orphaned and rebuilt on top of.
	 */
	static const FName GeneratedPartTag(TEXT("GeneratedPlantPart"));
}

// Sets default values
ABasePlant::ABasePlant()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	RootComponent = Body;
}

// Called when the game starts or when spawned
void ABasePlant::BeginPlay()
{
	Super::BeginPlay();

	// Delegate binding deliberately does NOT happen here. On the workbench path the plant is
	// spawned empty and only given its Item afterwards, so at BeginPlay there are no parts to
	// bind to. RebuildPartComponents owns binding instead.
}

void ABasePlant::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	RebuildPartComponents();
}

void ABasePlant::InitializeFromDefinition(const UPlantItemDefinition* InDef, const FItemInstanceData& InInstance)
{
	Item = InDef;
	Instance = InInstance;

	if (Item)
	{
		SetActorScale3D(Item->WorldScale);
	}

	RebuildPartComponents();
}

void ABasePlant::RebuildPartComponents()
{
	// --- Teardown ---------------------------------------------------------------------------
	TArray<UStaticMeshComponent*> ExistingComponents;
	GetComponents<UStaticMeshComponent>(ExistingComponents);
	for (UStaticMeshComponent* Comp : ExistingComponents)
	{
		if (Comp && Comp != Body && Comp->ComponentHasTag(GeneratedPartTag))
		{
			Comp->DestroyComponent();
		}
	}

	GeneratedPartComponents.Reset();
	PartComponentToHarvestRow.Reset();
	StructuralRow = INDEX_NONE;

	if (!Body)
	{
		return;
	}

	if (!Item)
	{
		Body->SetStaticMesh(nullptr);
		return;
	}

	// Editor preview must not accumulate delegate bindings across OnConstruction reruns.
	const bool bBindDelegates = GetWorld() && GetWorld()->IsGameWorld();

	// --- Build ------------------------------------------------------------------------------
	for (int32 RowIndex = 0; RowIndex < Item->HarvestableParts.Num(); ++RowIndex)
	{
		const FPlantPartHarvestData& Row = Item->HarvestableParts[RowIndex];
		const UDataAssetPlantPart* PartDef = Row.PlantPartDefinition;
		if (!PartDef)
		{
			continue;
		}

		UStaticMesh* RowMesh = Row.Mesh ? Row.Mesh.Get() : PartDef->WorldMesh.Get();

		// The structural row is the actor's own root body, not a separate component. It is never
		// added to PartComponentToHarvestRow, which is what makes it un-cuttable.
		if (Row.bIsStructural)
		{
			if (StructuralRow != INDEX_NONE)
			{
				UE_LOG(LogTemp, Warning, TEXT("[BasePlant] %s declares more than one structural part; using row %d."),
					*GetNameSafe(Item), StructuralRow);
				continue;
			}

			StructuralRow = RowIndex;
			Body->SetStaticMesh(RowMesh);
			if (PartDef->WorldMaterialOverride)
			{
				Body->SetMaterial(0, PartDef->WorldMaterialOverride);
			}
			continue;
		}

		for (int32 i = 0; i < Row.InstanceCount; ++i)
		{
			// NAME_None: a destroyed component lingers until GC, so reusing a stable name would
			// collide on rebuild and get silently renamed anyway.
			UStaticMeshComponent* Part = NewObject<UStaticMeshComponent>(this, NAME_None, RF_Transactional);
			if (!Part)
			{
				continue;
			}

			const FName Socket = Row.AttachSockets.IsValidIndex(i) ? Row.AttachSockets[i] : NAME_None;
			Part->SetupAttachment(Body, Socket);
			Part->ComponentTags.Add(GeneratedPartTag);
			Part->SetStaticMesh(RowMesh);

			// Parts ride the structural body; only the root simulates.
			Part->SetSimulatePhysics(false);
			// Set the profile before the per-channel override — SetCollisionProfileName resets
			// responses, so doing it the other way round would silently undo the Visibility block.
			Part->SetCollisionProfileName(TEXT("BlockAllDynamic"));
			Part->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			// Required for OnClicked / OnBeginCursorOver to fire on this component.
			Part->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

			// RegisterComponent is enough to make it render, including in the editor viewport.
			// Deliberately NOT AddInstanceComponent: that serializes generated geometry into the
			// .umap, so every OnConstruction rerun would dirty the level and reload would carry
			// stale copies that only the tag scan above saves us from.
			Part->RegisterComponent();

			if (Socket.IsNone() && Row.RelativeTransforms.IsValidIndex(i))
			{
				Part->SetRelativeTransform(Row.RelativeTransforms[i]);
			}

			if (PartDef->WorldMaterialOverride)
			{
				Part->SetMaterial(0, PartDef->WorldMaterialOverride);
			}

			if (bBindDelegates)
			{
				Part->OnBeginCursorOver.AddDynamic(this, &ABasePlant::HandleBeginCursorOver);
				Part->OnEndCursorOver.AddDynamic(this, &ABasePlant::HandleEndCursorOver);
				Part->OnClicked.AddDynamic(this, &ABasePlant::HandleClicked);
			}

			GeneratedPartComponents.Add(Part);
			PartComponentToHarvestRow.Add(Part, RowIndex);
		}
	}

	if (StructuralRow == INDEX_NONE)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BasePlant] %s has no structural harvest row — the plant has no body mesh."),
			*GetNameSafe(Item));
	}

	// Unique: Body survives a rebuild, so a second call would otherwise bind it twice and fire
	// the handlers twice per click. The generated parts are fresh objects and cannot double-bind.
	if (bBindDelegates && Body)
	{
		Body->OnBeginCursorOver.AddUniqueDynamic(this, &ABasePlant::HandleBeginCursorOver);
		Body->OnEndCursorOver.AddUniqueDynamic(this, &ABasePlant::HandleEndCursorOver);
		Body->OnClicked.AddUniqueDynamic(this, &ABasePlant::HandleClicked);
	}
}

void ABasePlant::SetPlantHighlight(bool bEnabled)
{
	UMaterialInterface* const Overlay =
		(bEnabled && HerbStatus == EHerbStatus::OnStand) ? OverlayMaterialInstance : nullptr;

	if (Body)
	{
		Body->SetOverlayMaterial(Overlay);
	}

	for (UStaticMeshComponent* Part : GeneratedPartComponents)
	{
		if (Part)
		{
			Part->SetOverlayMaterial(Overlay);
		}
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

bool ABasePlant::CanAcceptToolAction_Implementation(UToolItemDefinition* Tool, const FToolAction& Action, UPrimitiveComponent* HitComponent) const
{
	if (HerbStatus != EHerbStatus::OnTable) return false;
	if (!Item) return false;

	// Only harvestable parts are in the map. The structural body never is, so it is automatically
	// un-cuttable without the rule having to name a specific component.
	const int32* Row = PartComponentToHarvestRow.Find(Cast<UStaticMeshComponent>(HitComponent));
	if (!Row || !Item->HarvestableParts.IsValidIndex(*Row)) return false;

	// Which tools reach this part is the part's own data, not the plant's — so one species can
	// need a knife for its flower and pliers for its spines. Same gate APlantPart uses, so the
	// harvest moment and the on-the-table moment can never drift apart.
	const UDataAssetPlantPart* PartDef = Item->HarvestableParts[*Row].PlantPartDefinition;
	return PartDef && PartDef->AcceptsToolAction(Action);
}

void ABasePlant::ApplyToolActionResult_Implementation(UToolItemDefinition* Tool, const FToolAction& Action, const FMinigameResult& Result, UPrimitiveComponent* HitComponent)
{
	if (!Result.bSuccess)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BasePlant] Cutting failed"));
		return;
	}

	UStaticMeshComponent* CutPart = Cast<UStaticMeshComponent>(HitComponent);
	if (!CutPart || !Item) return;

	const int32* RowPtr = PartComponentToHarvestRow.Find(CutPart);
	if (!RowPtr || !Item->HarvestableParts.IsValidIndex(*RowPtr)) return;
	const FPlantPartHarvestData& Harvest = Item->HarvestableParts[*RowPtr];

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

	// HarvestChance / MinYield / MaxYield were declared on FPlantPartHarvestData but never read
	// until now — cutting used to always award exactly one of a hardcoded part.
	if (Inv && Harvest.PlantPartDefinition && FMath::FRand() <= Harvest.HarvestChance)
	{
		const int32 Yield = FMath::RandRange(Harvest.MinYield, FMath::Max(Harvest.MinYield, Harvest.MaxYield));
		if (Yield > 0)
		{
			Inv->AddItem(Harvest.PlantPartDefinition.Get(), Yield, NewInstance);
		}
	}

	CutPart->SetVisibility(false);
	CutPart->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PartComponentToHarvestRow.Remove(CutPart);

	// The structural part is what remains once everything else is gone; it is never cut directly.
	if (PartComponentToHarvestRow.IsEmpty())
	{
		if (Inv && Item->HarvestableParts.IsValidIndex(StructuralRow))
		{
			const FPlantPartHarvestData& Structural = Item->HarvestableParts[StructuralRow];
			if (Structural.PlantPartDefinition)
			{
				const int32 Yield = FMath::RandRange(Structural.MinYield, FMath::Max(Structural.MinYield, Structural.MaxYield));
				if (Yield > 0)
				{
					Inv->AddItem(Structural.PlantPartDefinition.Get(), Yield, NewInstance);
				}
			}
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
