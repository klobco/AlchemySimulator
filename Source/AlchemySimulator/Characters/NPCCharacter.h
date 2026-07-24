// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NPCCharacter.generated.h"

UCLASS()
class ALCHEMYSIMULATOR_API ANPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANPCCharacter();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
    bool bShouldGoToWork = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UPatientConditionComponent* PatientConditionComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
