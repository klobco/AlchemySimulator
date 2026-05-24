// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IInteractable.h"
#include "ItemMetadata.h"
#include "PlantItemDefinition.h"
#include "BasePlant.generated.h"

UCLASS()
class ALCHEMYSIMULATOR_API ABasePlant : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABasePlant();

	UPROPERTY(EditAnywhere, Category = "components")
	class UStaticMeshComponent* Stem;

	UPROPERTY(EditAnywhere, Category = "components")
	UStaticMeshComponent* Leaf_A;

	UPROPERTY(EditAnywhere, Category = "components")
	UStaticMeshComponent* Leaf_B;

	UPROPERTY(EditAnywhere, Category = "components")
	UStaticMeshComponent* Fruit;

	UPROPERTY(EditDefaultsOnly, Category = "components")
	int32 LeafCount = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<const UPlantItemDefinition> Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<const UPlantItemDefinition> StemItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<const UPlantItemDefinition> LeafItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<const UPlantItemDefinition> FruitItem;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	UStaticMeshComponent* InteractedPart;

	UPROPERTY(EditDefaultsOnly, Category = "Minigames")
	TSubclassOf<class UAlchemyCutMinigameWidget> CuttingMinigameWidgetClass;

	UPROPERTY()
	class ABasicWorkbench* ParentWorkbench;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemInstanceData Instance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* OverlayMaterialInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	EHerbStatus HerbStatus;

	UFUNCTION()
	void HandleBeginCursorOver(UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleClicked(UPrimitiveComponent* Component, FKey ButtonPressed);

	UFUNCTION()
	void OnCuttingFinished(bool bSuccess);

	void SetPlantHighlight(bool bEnabled);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual FText   GetInteractPrompt_Implementation() const override;
	virtual FVector GetInteractWorldLocation_Implementation() const override;
	virtual bool    CanInteract_Implementation(APawn* By) const override;
	virtual void    Interact_Implementation(APawn* By) override;
	virtual void    OnFocStart_Implementation(APawn* By) override;
	virtual void    OnFocEnd_Implementation(APawn* By) override;

};
