// Fill out your copyright notice in the Description page of Project Settings.


#include "PlantPart.h"
#include "AlchemySimulatorCharacter.h"
#include "AlchemySimulatorPlayerController.h"
#include "InventoryComponent.h"
#include "BasicWorkbench.h"
#include "BaseTool.h"
#include "DataAssetProcessingMethod.h"
#include "ToolItemDefinition.h"
#include "PestleMortarMinigame.h"
#include "MinigameManagerComponent.h"

// Sets default values
APlantPart::APlantPart()
{
 	PrimaryActorTick.bCanEverTick = false;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	RootComponent = Body;
}

// Called when the game starts or when spawned
void APlantPart::BeginPlay()
{
	Super::BeginPlay();

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

	if (ParentWorkbench->ActiveToolIndex == INDEX_NONE)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (!PC) return;
		AAlchemySimulatorPlayerController* MyPC = Cast<AAlchemySimulatorPlayerController>(PC);
		if (!MyPC) return;
		MyPC->StartWorldDrag(this);
		return;
	}

	ABaseTool* Tool = *ParentWorkbench->Tools.Find(ParentWorkbench->ActiveToolIndex);
	if (!Tool || !Tool->Item) return;

	if (Tool->Item->ToolCategory == EToolCategory::Pestle)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (!PC) return;
		AAlchemySimulatorPlayerController* MyPC = Cast<AAlchemySimulatorPlayerController>(PC);
		if (!MyPC || !MyPC->MinigameManager) return;

		MyPC->MinigameManager->OnMinigameFinished.AddDynamic(this, &APlantPart::OnPestleFinished);
		MyPC->MinigameManager->StartMinigame(PestleMortarMinigameWidgetClass);
	}
}

void APlantPart::OnPestleFinished(bool bSuccess)
{
    if (AAlchemySimulatorPlayerController* MyPC = Cast<AAlchemySimulatorPlayerController>(GetWorld()->GetFirstPlayerController()))
    {
        MyPC->MinigameManager->OnMinigameFinished.RemoveDynamic(this, &APlantPart::OnPestleFinished);
    }

    if (!bSuccess)
    {
        UE_LOG(LogTemp, Warning, TEXT("[PlantPart] Pestle failed"));
        return;
    }
    FItemInstanceData NewInstance = Instance;
    NewInstance.bIsProcessed = true;
    if (PestleProcessingMethod)
	{
		NewInstance.ProcessingTags.AddTag(PestleProcessingMethod->ProcessingTag);
		NewInstance.ProcessingQuality *= PestleProcessingMethod->GeneralPotencyMultiplier;
	}
    NewInstance.ProcessingQuality *= 1.15f;

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

    ParentWorkbench->RemoveHerbItem(this);
    Destroy();
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
