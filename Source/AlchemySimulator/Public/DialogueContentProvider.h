#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Dialogue/AlchemyDialogueTypes.h"
#include "DialogueContentProvider.generated.h"



UINTERFACE(BlueprintType)
class ALCHEMYSIMULATOR_API UDialogueContentProvider : public UInterface
{
    GENERATED_BODY()
};


/**
 * Supplies the content of one conversation, one step at a time.
 *
 * Lifetime: the runtime creates one instance per conversation and drops it at the end.
 *
 * Contract:
 *   BeginConversation() -> GetCurrentStep() -> [Advance() | SelectOption()]* -> EndConversation()
 *
 * Rules for any implementation:
 *   - never reference a widget, a quest class, or a reputation class
 *   - never mutate world state directly; put changes in FDialogueStep::Effects
 *   - return an already-filtered Options array
 */
class ALCHEMYSIMULATOR_API IDialogueContentProvider
{
    GENERATED_BODY()
public:
    /** False = "I have nothing to say in this context"; the runtime then aborts cleanly. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Dialogue")
    bool BeginConversation(const FAlchemyDialogueContext& Context);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Dialogue")
    FDialogueStep GetCurrentStep() const;

    /** Move past a Line step. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Dialogue")
    FDialogueStep Advance();

    /** Answer a Choices step. Index is into the step's filtered Options array. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Dialogue")
    FDialogueStep SelectOption(int32 Index);

    /** Called on normal completion and on abort. */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Dialogue")
    void EndConversation(bool bAborted);
};