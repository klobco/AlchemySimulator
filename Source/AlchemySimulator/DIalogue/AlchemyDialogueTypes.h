#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "GameplayTagContainer.h"
#include "AlchemyDialogueTypes.generated.h"



USTRUCT(BlueprintType)
struct FDialogueEffects
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagContainer FlagsToAdd;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagContainer FlagsToRemove;

    /** e.g. Reputation.Greta -> +5 */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TMap<FGameplayTag, float> NumberDeltas;

    /** Broadcast on the runtime for anything that needs a callback (give item, play cutscene) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagContainer EventsToFire;
};

USTRUCT(BlueprintType)
struct FDialogueOption
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText Text;              // what the player says
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName TargetNodeID;      // where it goes
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagQuery Condition;   
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FDialogueEffects Effects;     
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bEndsDialogue = false;
};

USTRUCT(BlueprintType)
struct FDialogueNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName NodeID;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTag SpeakerTag;   // Speaker.NPC / Speaker.Player
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true)) FText Line;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USoundBase>    VoiceLine;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UAnimMontage>  Gesture;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagQuery Condition;  
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FDialogueEffects Effects;     

    /** Used when Options is empty: auto-advance target. NAME_None = end. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName NextNodeID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FDialogueOption> Options;
};

USTRUCT(BlueprintType)
struct FDialogueEntryPoint
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagQuery Condition;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName StartNodeID;
};