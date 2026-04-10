// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "ItemMetadata.generated.h"

class UItemDefinitionBase;


USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FItemInstanceData
{
    GENERATED_BODY()

public:
    // Zatia¾ nechaj prázdne alebo sem neskôr pridaj:
    // Quality, Freshness, Durability, CraftedBy, etc.
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quality = 100;
};

USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FInventorySlot
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TObjectPtr<const UItemDefinitionBase> Item = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0"))
    int32 Quantity = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FItemInstanceData Instance;

    bool IsEmpty() const { return Item == nullptr || Quantity <= 0; }
};