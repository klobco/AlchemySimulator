// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IInteractable.h"
#include "ItemMetadata.h"
#include "DataAssets/DataAssetPlantPart.h"
#include "PlantPart.generated.h"

UCLASS()
class ALCHEMYSIMULATOR_API APlantPart : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APlantPart();

	UPROPERTY(EditAnywhere, Category = "Components")
	UStaticMeshComponent* Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemInstanceData Instance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	TObjectPtr<const UDataAssetPlantPart> Item;

	UPROPERTY(EditDefaultsOnly, Category = "Minigames")
	TSubclassOf<class UPestleMortarMinigame> PestleMortarMinigameWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* OverlayMaterialInstance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
	EHerbStatus HerbStatus;

	UPROPERTY(EditDefaultsOnly, Category="Alchemy|Processing")
	TObjectPtr<UDataAssetProcessingMethod> PestleProcessingMethod;

	UPROPERTY()
	class ABasicWorkbench* ParentWorkbench = nullptr;

	UFUNCTION()
	void HandleBeginCursorOver(UPrimitiveComponent* Component);

	UFUNCTION()
	void OnPestleFinished(bool bSuccess);

	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleClicked(UPrimitiveComponent* Component, FKey ButtonPressed);

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
