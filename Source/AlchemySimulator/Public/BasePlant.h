// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IInteractable.h"
#include "ItemMetadata.h"
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	class UItemDefinitionBase* Item;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	FItemInstanceData Instance;

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
