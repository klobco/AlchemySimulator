// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Dialogue/AlchemyDialogueTypes.h"
#include "DialogueDataAsset.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDialogueDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
    /** Checked top to bottom; first match wins. Put the most specific first. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue")
    TArray<FDialogueEntryPoint> EntryPoints;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Dialogue",
              meta=(TitleProperty="NodeID"))
    TArray<FDialogueNode> Nodes;

    const FDialogueNode* FindNode(FName ID) const;

#if WITH_EDITOR
    // NOTE: const qualifier is required in UE5.
    virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;  // Step 12
#endif
};
