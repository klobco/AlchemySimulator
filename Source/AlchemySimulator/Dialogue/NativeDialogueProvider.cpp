// Fill out your copyright notice in the Description page of Project Settings.

#include "NativeDialogueProvider.h"
#include "DataAssets/DialogueDataAsset.h"
#include "Subsystems/WorldStateSubsystem.h"
#include "Characters/NPCCharacter.h"
#include "NativeDialogueProvider.h"

/** Returns true when there's no condition, and before Step 6 exists at all. */
bool UNativeDialogueProvider::PassesCondition(const FGameplayTagQuery& Q) const
{
    if (Q.IsEmpty())      return true;
    if (!Ctx.WorldState)  return true;      // Step 6 lights this up; harmless until then
    return Ctx.WorldState->MatchesQuery(Q);
}

bool UNativeDialogueProvider::BeginConversation_Implementation(const FAlchemyDialogueContext& Context)
{
    Ctx = Context;

    if (!Asset && Context.Speaker)
    {
        Asset = Context.Speaker->Dialogue;
    }

    if (!Asset || Asset->Nodes.IsEmpty()) return false;

    UE_LOG(LogTemp, Warning, TEXT("Beginning conversation with NPC, using asset: %s"), *GetNameSafe(Asset));
    for (const FDialogueEntryPoint& EP : Asset->EntryPoints)
    {
        if (PassesCondition(EP.Condition) && Asset->FindNode(EP.StartNodeID))
        {
            CurrentStep = BuildStepFor(EP.StartNodeID);
            return true;
        }
    }
    return false;   // no entry point matched: "I have nothing to say right now"
}

FDialogueStep UNativeDialogueProvider::GetCurrentStep_Implementation() const
{
    return CurrentStep;
}

FDialogueStep UNativeDialogueProvider::BuildStepFor(FName NodeID)
{
    FDialogueStep Step;   // Type defaults to End

    const FDialogueNode* Node = Asset->FindNode(NodeID);
    if (!Node)
    {
        UE_LOG(LogTemp, Error, TEXT("Dialogue: '%s' has no node '%s' — ending conversation"),
               *GetNameSafe(Asset), *NodeID.ToString());
        return Step;      // never crash on bad data; Step 12's IsDataValid catches it in-editor
    }

    UE_LOG(LogTemp, Warning, TEXT("Dialogue: '%s' advancing to node '%s'"), *GetNameSafe(Asset), *NodeID.ToString());
    CurrentNodeID = NodeID;
    Step.Node     = *Node;
    Step.Effects  = Node->Effects;

    for (const FDialogueOption& Opt : Node->Options)
    {
        if (PassesCondition(Opt.Condition))
        {
            Step.Options.Add(Opt);
        }
    }

    Step.Type = Step.Options.IsEmpty() ? EDialogueStepType::Line : EDialogueStepType::Choices;
    return Step;
}

FDialogueStep UNativeDialogueProvider::Advance_Implementation()
{
    const FDialogueNode* Node = Asset->FindNode(CurrentNodeID);
    const FName Next = Node ? Node->NextNodeID : NAME_None;

    CurrentStep = (Next == NAME_None) ? FDialogueStep() : BuildStepFor(Next);
    return CurrentStep;
}

FDialogueStep UNativeDialogueProvider::SelectOption_Implementation(int32 Index)
{
    if (!CurrentStep.Options.IsValidIndex(Index))
    {
        CurrentStep = FDialogueStep();
        return CurrentStep;
    }

    // Copy, not a reference — CurrentStep is about to be overwritten underneath us.
    const FDialogueOption Chosen = CurrentStep.Options[Index];

    CurrentStep = (Chosen.bEndsDialogue || Chosen.TargetNodeID == NAME_None)
                ? FDialogueStep()
                : BuildStepFor(Chosen.TargetNodeID);

    // The option's own effects ride along with whatever step it leads to,
    // including the End step — otherwise a conversation-ending choice silently
    // loses its consequences.
    CurrentStep.Effects.FlagsToAdd.AppendTags(Chosen.Effects.FlagsToAdd);
    CurrentStep.Effects.FlagsToRemove.AppendTags(Chosen.Effects.FlagsToRemove);
    CurrentStep.Effects.EventsToFire.AppendTags(Chosen.Effects.EventsToFire);
    for (const TPair<FGameplayTag, float>& Delta : Chosen.Effects.NumberDeltas)
    {
        CurrentStep.Effects.NumberDeltas.FindOrAdd(Delta.Key) += Delta.Value;
    }

    return CurrentStep;
}

void UNativeDialogueProvider::EndConversation_Implementation(bool bAborted)
{
    // Nothing to flush: every world change already left via FDialogueStep::Effects.
    // The Inkpot provider in Step 13 is not so lucky — see the sync contract there.
}