#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectPtr.h"
#include "GameplayTagContainer.h"
#include "DataStructHelpers.h"
#include "AlchemyDialogueTypes.generated.h"

UENUM(BlueprintType)
enum class EDialogueStepType : uint8
{
    Line,      // display Node; advance on player input
    Choices,   // display Node + Options; wait for a selection
    End
};

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
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagQuery Condition;   // Step 6
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FDialogueEffects Effects;      // Step 7
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FGameplayTagQuery Condition;  // Step 6
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FDialogueEffects Effects;     // Step 7

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




/**
 * The entire contract between the runtime and any content backend.
 * Deliberately made of plain data — no UObjects, no engine coupling —
 * so that a text-driven backend (Step 13) can produce it just as easily
 * as a Data Asset can.
 */
USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FDialogueStep
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) EDialogueStepType Type = EDialogueStepType::End;

    /** Valid for Line and Choices. */
    UPROPERTY(BlueprintReadOnly) FDialogueNode Node;

    /** Already filtered. The UI must index into THIS, never into Node.Options. */
    UPROPERTY(BlueprintReadOnly) TArray<FDialogueOption> Options;

    /** Applied by the runtime before the step is broadcast. */
    UPROPERTY(BlueprintReadOnly) FDialogueEffects Effects;
};


/** Everything a provider needs to know about the situation. */
USTRUCT(BlueprintType)
struct ALCHEMYSIMULATOR_API FAlchemyDialogueContext
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) TObjectPtr<class ANPCCharacter> Speaker = nullptr;
    UPROPERTY(BlueprintReadOnly) TObjectPtr<APawn> Player = nullptr;

    /** Injected by the runtime so providers never fetch it themselves. Null until Step 6. */
    UPROPERTY(BlueprintReadOnly) TObjectPtr<class UWorldStateSubsystem> WorldState = nullptr;

    /** Symptoms, disease tags, NPC identity — populated in Step 6. */
    UPROPERTY(BlueprintReadOnly) FGameplayTagContainer ContextTags;
};