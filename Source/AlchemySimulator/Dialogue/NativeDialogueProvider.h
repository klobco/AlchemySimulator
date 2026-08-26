// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DialogueContentProvider.h"
#include "Dialogue/AlchemyDialogueTypes.h"
#include "NativeDialogueProvider.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UNativeDialogueProvider : public UObject, public IDialogueContentProvider
{
	GENERATED_BODY()
	
public:
    /** Set by the caller before BeginConversation. */
    UPROPERTY() TObjectPtr<class UDialogueDataAsset> Asset = nullptr;

    virtual bool BeginConversation_Implementation(const FAlchemyDialogueContext& Context) override;
    virtual FDialogueStep GetCurrentStep_Implementation() const override;
    virtual FDialogueStep Advance_Implementation() override;
    virtual FDialogueStep SelectOption_Implementation(int32 Index) override;
    virtual void EndConversation_Implementation(bool bAborted) override;

private:
    bool PassesCondition(const FGameplayTagQuery& Q) const;
    FDialogueStep BuildStepFor(FName NodeID);

    FAlchemyDialogueContext Ctx;
    FName CurrentNodeID;
    FDialogueStep CurrentStep;
};
