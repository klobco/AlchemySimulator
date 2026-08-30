// Fill out your copyright notice in the Description page of Project Settings.


#include "Dialogue/NativeDialogueProvider.h"
#include "Dialogue/AlchemyDialogueTypes.h"
#include "Subsystems/WorldStateSubsystem.h"
#include "Characters/NPCCharacter.h"
#include "DialogueRuntimeComponent.h"

// Sets default values for this component's properties
UDialogueRuntimeComponent::UDialogueRuntimeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

}

bool UDialogueRuntimeComponent::StartDialogue(ANPCCharacter* NPC, TSubclassOf<UObject> ProviderClass)
{
	if (!NPC) return false;

    if (IsInDialogue() || !ProviderClass) return false;
    if (!ProviderClass->ImplementsInterface(UDialogueContentProvider::StaticClass())) {
        UE_LOG(LogTemp, Error, TEXT("Provider class does not implement UDialogueContentProvider interface"));
        return false;
    }

	UE_LOG(LogTemp, Warning, TEXT("Starting dialogue with NPC: %s using provider class: %s"), *GetNameSafe(NPC), *GetNameSafe(ProviderClass));

    UObject* Provider = NewObject<UObject>(this, ProviderClass);
    FAlchemyDialogueContext Ctx = { NPC, Cast<APlayerController>(GetOwner())->GetPawn(), GetWorld()->GetGameInstance()->GetSubsystem<UWorldStateSubsystem>()};
    if (!IDialogueContentProvider::Execute_BeginConversation(Provider, Ctx)) {
        return false;
    }

	UE_LOG(LogTemp, Warning, TEXT("Dialogue started successfully with NPC: %s"), *GetNameSafe(NPC));

    ActiveProvider = Provider; CurrentNPC = NPC;
    NPC->BeginConversation(Ctx.Player);
    OnDialogueStarted.Broadcast(NPC);

	// Take the entry step the provider built during BeginConversation. Without this
	// HandleStep sees a default FDialogueStep, whose Type is End, and ends instantly.
	HandleStep(IDialogueContentProvider::Execute_GetCurrentStep(Provider));
	return true;
}

bool UDialogueRuntimeComponent::StartDialogueFromAsset(ANPCCharacter* NPC, UDialogueDataAsset* Asset)
{
	if (!NPC || !Asset)
		return false;

	UObject* Provider = NewObject<UNativeDialogueProvider>(this);
	if (!Provider)
		return false;

	Cast<UNativeDialogueProvider>(Provider)->Asset = Asset;
	IDialogueContentProvider::Execute_BeginConversation(Provider, FAlchemyDialogueContext{ NPC, Cast<APlayerController>(GetOwner())->GetPawn(), GetWorld()->GetGameInstance()->GetSubsystem<UWorldStateSubsystem>() });
	CurrentNPC = NPC;
	ActiveProvider = Provider;
	CurrentStep = IDialogueContentProvider::Execute_GetCurrentStep(ActiveProvider);

	HandleStep(CurrentStep);
	return true;
}

void UDialogueRuntimeComponent::Advance()
{
	if (!ActiveProvider)
		return;

	UE_LOG(LogTemp, Warning, TEXT("Advancing dialogue with NPC: %s"), *GetNameSafe(CurrentNPC));
	if (!IsInDialogue() || CurrentStep.Type == EDialogueStepType::Choices) return;
	HandleStep(IDialogueContentProvider::Execute_Advance(ActiveProvider));
}

void UDialogueRuntimeComponent::SelectOption(int32 Index)
{
	if (!ActiveProvider)
		return;

	UE_LOG(LogTemp, Warning, TEXT("Selecting option %d in dialogue with NPC: %s"), Index, *GetNameSafe(CurrentNPC));
	if (!IsInDialogue() || CurrentStep.Type != EDialogueStepType::Choices) return;
    if (!CurrentStep.Options.IsValidIndex(Index)) return;

	HandleStep(IDialogueContentProvider::Execute_SelectOption(ActiveProvider, Index));
}

void UDialogueRuntimeComponent::EndDialogue()
{
	if (!ActiveProvider)
		return;

	if (!IsInDialogue()) return;

	UE_LOG(LogTemp, Warning, TEXT("Ending dialogue with NPC: %s"), *GetNameSafe(CurrentNPC));

	IDialogueContentProvider::Execute_EndConversation(ActiveProvider, CurrentStep.Type != EDialogueStepType::End);
	CurrentNPC->EndConversation();
	ActiveProvider = nullptr;
	CurrentNPC = nullptr;
	OnDialogueEnded.Broadcast();
}

void UDialogueRuntimeComponent::HandleStep(const FDialogueStep& Step)
{
	CurrentStep = Step;
	ApplyEffects(Step.Effects);
	OnStepEntered.Broadcast(Step);

	UE_LOG(LogTemp, Warning, TEXT("Handling dialogue step: NodeID=%s, Type=%d, OptionsCount=%d"), *Step.Node.NodeID.ToString(), (int32)Step.Type, Step.Options.Num());

	if (Step.Type == EDialogueStepType::End)
	{
		EndDialogue();
	}
}

 void UDialogueRuntimeComponent::ApplyEffects(const FDialogueEffects& Effects){
	return;   // Step 7; no-op for now
 }