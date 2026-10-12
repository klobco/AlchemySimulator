// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemMetadata.h"
#include "IInteractable.h"
#include "ItemDefinitions/ToolItemDefinition.h"
#include "BaseTool.generated.h"

UCLASS()
class ALCHEMYSIMULATOR_API ABaseTool : public AActor, public IInteractable
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABaseTool();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "tool")
	class UStaticMeshComponent* ToolMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "tool")
	UToolItemDefinition* Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "tool")
	FItemInstanceData Instance;

	/**
	 * How many inventory units this one actor stands for. 1 everywhere except a stack dropped
	 * out of an inventory, which spawns a single actor carrying the whole count.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "tool", meta = (ClampMin = "1"))
	int32 Quantity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInstance* OverlayMaterialInstance;

	UFUNCTION()
	void HandleBeginCursorOver(class UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleEndCursorOver(UPrimitiveComponent* Component);

	UFUNCTION()
	void HandleClicked(UPrimitiveComponent* Component, FKey ButtonPressed);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:

	virtual FText   GetInteractPrompt_Implementation() const override;
	virtual FVector GetInteractWorldLocation_Implementation() const override;
	virtual bool    CanInteract_Implementation(APawn* By) const override;
	virtual void    Interact_Implementation(APawn* By) override;
	virtual void    OnFocStart_Implementation(APawn* By) override;
	virtual void    OnFocEnd_Implementation(APawn* By) override;

};
