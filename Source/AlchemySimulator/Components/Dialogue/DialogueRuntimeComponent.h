// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Dialogue/AlchemyDialogueTypes.h"
#include "DialogueRuntimeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStarted, class ANPCCharacter*, Speaker);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueStep, const FDialogueStep&, Step);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);


UCLASS( ClassGroup=(Dialogue), meta=(BlueprintSpawnableComponent) )
class ALCHEMYSIMULATOR_API UDialogueRuntimeComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UDialogueRuntimeComponent();
    /**
     * @param ProviderClass  any UObject class implementing IDialogueContentProvider.
     *                       Configure the instance via the OnProviderCreated hook below,
     *                       or use the convenience overload for the common case.
     */
    UFUNCTION(BlueprintCallable) bool StartDialogue(ANPCCharacter* NPC, TSubclassOf<UObject> ProviderClass);

    /** Convenience: creates a UNativeDialogueProvider pointed at Asset. */
    UFUNCTION(BlueprintCallable) bool StartDialogueFromAsset(ANPCCharacter* NPC, class UDialogueDataAsset* Asset);

    UFUNCTION(BlueprintCallable) void Advance();               // "next line" for option-less steps
    UFUNCTION(BlueprintCallable) void SelectOption(int32 Index);
    UFUNCTION(BlueprintCallable) void EndDialogue();
    UFUNCTION(BlueprintPure)     bool IsInDialogue() const { return ActiveProvider != nullptr; }
    UFUNCTION(BlueprintPure)     const FDialogueStep& GetCurrentStep() const { return CurrentStep; }

    UPROPERTY(BlueprintAssignable) FOnDialogueStarted OnDialogueStarted;
    UPROPERTY(BlueprintAssignable) FOnDialogueStep    OnStepEntered;
    UPROPERTY(BlueprintAssignable) FOnDialogueEnded   OnDialogueEnded;

private:
    void HandleStep(const FDialogueStep& Step);
    void ApplyEffects(const FDialogueEffects& Effects);   // Step 7; no-op for now

    UPROPERTY() TObjectPtr<UObject>       ActiveProvider = nullptr;
    UPROPERTY() TObjectPtr<ANPCCharacter> CurrentNPC     = nullptr;
    FDialogueStep CurrentStep;
};
