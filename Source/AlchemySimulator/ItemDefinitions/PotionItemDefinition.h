// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "PotionItemDefinition.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UPotionItemDefinition : public UItemDefinitionBase
{
	GENERATED_BODY()

public:

	UPotionItemDefinition();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Potion")
    TObjectPtr<UStaticMesh> BottleMesh = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Potion")
    TObjectPtr<UStaticMesh> LiquidMesh = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Potion")
    FName LiquidColorParameterName = TEXT("LiquidColor");

	virtual bool CanUseItem_Implementation(const FItemUseContext& Context) const override;
	virtual FItemUseResult UseItem_Implementation(const FItemUseContext& Context) const override;
};
