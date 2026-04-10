// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IInteractable.h"
#include "BasicInteractableStationObject.generated.h"

UCLASS()
class ALCHEMYSIMULATOR_API ABasicInteractableStationObject : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasicInteractableStationObject();

	UPROPERTY(EditAnywhere, Category = "tools")
	class UInventoryComponent* herbsInventory;

	UPROPERTY(EditAnywhere, Category = "tools")
	UInventoryComponent* toolsInventory;

	UPROPERTY(EditAnywhere, Category = "tools")
	TMap<int32, class ABaseTool* > Tools;

	UPROPERTY(VisibleAnywhere, Category = "tools")
	int32 ActiveToolIndex;

	UPROPERTY(EditAnywhere, Category = "tools")
	int32 ToolsLimit;

	UPROPERTY(EditAnywhere, Category = "tools")
	int32 HerbsLimit;


	UFUNCTION()
	void HandleBeginCursorOver(class UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleClicked(UPrimitiveComponent* Component, FKey ButtonPressed);

	UFUNCTION()
	void OpenInventory(UPrimitiveComponent* Component, FKey ButtonPressed);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* OverlayMaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "widgets")
	TSubclassOf<class UTableInventoryWidget> TableInventoryWidgetClass;


	virtual void AddTool(ABaseTool* tool);

	virtual void RemoveTool(ABaseTool* tool);

	virtual void SetActiveTool(ABaseTool* tool);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, Category = "components")
	USceneComponent* InteractionViewPoint;

	UPROPERTY(EditAnywhere, Category = "components")
	UStaticMeshComponent* Body;

	UPROPERTY(EditAnywhere, Category = "components")
	UStaticMeshComponent* MenuMesh;

	UPROPERTY(EditAnywhere, Category = "components")
	UStaticMeshComponent* HerbMesh;

	UPROPERTY(EditAnywhere, Category = "widgets")
	TSubclassOf<UUserWidget> TableScreen = nullptr;

	virtual FText   GetInteractPrompt_Implementation() const override;
	virtual FVector GetInteractWorldLocation_Implementation() const override;
	virtual bool    CanInteract_Implementation(APawn* By) const override;
	virtual void    Interact_Implementation(APawn* By) override;
	virtual void    OnFocStart_Implementation(APawn* By) override;
	virtual void    OnFocEnd_Implementation(APawn* By) override;

};
