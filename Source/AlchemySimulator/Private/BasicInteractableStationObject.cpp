// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicInteractableStationObject.h"
#include "Components/StaticMeshComponent.h"
#include "BaseTool.h"
#include "TableWidget.h"
#include "AlchemySimulatorPlayerController.h"
#include "Components/SceneComponent.h"
#include "TableInventoryWidget.h"
#include "InventoryComponent.h"
#include "BaseGameWidget.h"
#include "WidgetStackManager.h"

// Sets default values
ABasicInteractableStationObject::ABasicInteractableStationObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh body"));
	RootComponent = Body;

	MenuMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Menu Mesh"));
	MenuMesh->SetupAttachment(RootComponent);

	HerbMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Herb Mesh"));
	HerbMesh->SetupAttachment(RootComponent);

	InteractionViewPoint = CreateDefaultSubobject<USceneComponent>(TEXT("camera hold"));
	InteractionViewPoint->SetupAttachment(RootComponent);

	toolsInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Tools Inventory"));
	herbsInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Herbs Inventory"));

}

// Called when the game starts or when spawned
void ABasicInteractableStationObject::BeginPlay()
{
	Super::BeginPlay();

	MenuMesh->OnBeginCursorOver.AddDynamic(this, &ABasicInteractableStationObject::HandleBeginCursorOver);
	MenuMesh->OnEndCursorOver.AddDynamic(this, &ABasicInteractableStationObject::HandleEndCursorOver);
	MenuMesh->OnClicked.AddDynamic(this, &ABasicInteractableStationObject::HandleClicked);

	HerbMesh->OnBeginCursorOver.AddDynamic(this, &ABasicInteractableStationObject::HandleBeginCursorOver);
	HerbMesh->OnEndCursorOver.AddDynamic(this, &ABasicInteractableStationObject::HandleEndCursorOver);
	HerbMesh->OnClicked.AddDynamic(this, &ABasicInteractableStationObject::OpenInventory);
	
}

void ABasicInteractableStationObject::HandleBeginCursorOver(UPrimitiveComponent* Component)
{

	if (Component == HerbMesh)
	{
		HerbMesh->SetOverlayMaterial(OverlayMaterialInstance);
	}
	else
	{
		MenuMesh->SetOverlayMaterial(OverlayMaterialInstance);
	}
}

void ABasicInteractableStationObject::HandleEndCursorOver(UPrimitiveComponent* Component)
{
	if (Component == HerbMesh)
	{
		HerbMesh->SetOverlayMaterial(nullptr);
	}
	else
	{
		MenuMesh->SetOverlayMaterial(nullptr);
	}
}

void ABasicInteractableStationObject::HandleClicked(UPrimitiveComponent* Component, FKey ButtonPressed)
{
	AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC || !TableScreen) return;

	// Toggle: if a UTableWidget is already on top of the stack, pop it
	if (UBaseGameWidget* Top = PC->WidgetManager->GetTopWidget())
	{
		if (Top->IsA<UTableWidget>())
		{
			PC->PopWidget();
			return;
		}
	}

	UTableWidget* Widget = Cast<UTableWidget>(CreateWidget<UUserWidget>(PC, TableScreen));
	if (!Widget) return;

	Widget->SetTable(this);
	PC->PushWidget(Widget);
}

void ABasicInteractableStationObject::OpenInventory(UPrimitiveComponent* Component, FKey ButtonPressed)
{
	AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC || !TableInventoryWidgetClass) return;

	UTableInventoryWidget* Widget = CreateWidget<UTableInventoryWidget>(PC, TableInventoryWidgetClass);
	if (!Widget) return;

	Widget->Setup(this);
	PC->PushWidget(Widget);
}

// Called every frame
void ABasicInteractableStationObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


FText ABasicInteractableStationObject::GetInteractPrompt_Implementation() const
{
	return FText::GetEmpty();
}

FVector ABasicInteractableStationObject::GetInteractWorldLocation_Implementation() const
{
	return GetActorLocation();
}

bool ABasicInteractableStationObject::CanInteract_Implementation(APawn* By) const
{
	return true;
}

void ABasicInteractableStationObject::Interact_Implementation(APawn* By)
{

	if (!By || !TableScreen) return;

	//if (AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(By->GetController()))
	//{
	//	PC->OpenWidget(TableScreen);
	//}
}

void ABasicInteractableStationObject::OnFocStart_Implementation(APawn* By)
{

}

void ABasicInteractableStationObject::OnFocEnd_Implementation(APawn* By)
{

}

void ABasicInteractableStationObject::AddTool(ABaseTool* tool) {


}

void ABasicInteractableStationObject::RemoveTool(ABaseTool* tool) {
	//toolsInventory.Remove(tool);
}

void ABasicInteractableStationObject::SetActiveTool(ABaseTool* tool) {
	if (!tool)
	{
		ActiveToolIndex = INDEX_NONE;
		return;
	}

	for (auto& Pair : Tools)
	{
		if (Pair.Value == tool)
		{
			ActiveToolIndex = Pair.Key;
			return;
		}
	}
}