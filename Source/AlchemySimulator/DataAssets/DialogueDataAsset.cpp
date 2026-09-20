// Fill out your copyright notice in the Description page of Project Settings.


#include "DialogueDataAsset.h"

EDataValidationResult UDialogueDataAsset::IsDataValid(
    FDataValidationContext& Context) const
{
    return Super::IsDataValid(Context);
}

const FDialogueNode* UDialogueDataAsset::FindNode(FName ID) const
{
    for (const FDialogueNode& Node : Nodes)
    {
        if (Node.NodeID == ID)
        {
            return &Node;
        }
    }
    return nullptr;
}