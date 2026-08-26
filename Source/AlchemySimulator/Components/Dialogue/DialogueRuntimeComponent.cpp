// Fill out your copyright notice in the Description page of Project Settings.


#include "Dialogue/NativeDialogueProvider.h"
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
	if (!NPC || !ProviderClass)
		return false;

	UObject* Provider = NewObject<UObject>(this, ProviderClass);
	if (!Provider || !Provider->GetClass()->ImplementsInterface(UDialogueContentProvider::StaticClass()))
		return false;

	IDialogueContentProvider::Execute_BeginConversation(Provider, FAlchemyDialogueContext{ NPC, Cast<APawn>(GetOwner()) });
	CurrentNPC = NPC;
	ActiveProvider = Provider;
	CurrentStep = IDialogueContentProvider::Execute_GetCurrentStep(ActiveProvider);

	HandleStep(CurrentStep);
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
	IDialogueContentProvider::Execute_BeginConversation(Provider, FAlchemyDialogueContext{ NPC, Cast<APawn>(GetOwner()) });
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

	CurrentStep = IDialogueContentProvider::Execute_Advance(ActiveProvider);
	HandleStep(CurrentStep);
}

void UDialogueRuntimeComponent::SelectOption(int32 Index)
{
	if (!ActiveProvider)
		return;

	CurrentStep = IDialogueContentProvider::Execute_SelectOption(ActiveProvider, Index);
	HandleStep(CurrentStep);
}

void UDialogueRuntimeComponent::EndDialogue()
{
	if (!ActiveProvider)
		return;

	IDialogueContentProvider::Execute_EndConversation(ActiveProvider, false);
	ActiveProvider = nullptr;
	CurrentNPC = nullptr;
	OnDialogueEnded.Broadcast();
}

void UDialogueRuntimeComponent::HandleStep(const FDialogueStep& Step)
{
	ApplyEffects(Step.Effects);
	OnStepEntered.Broadcast(Step);

	if (Step.Type == EDialogueStepType::End)
	{
		EndDialogue();
	}
}

 void UDialogueRuntimeComponent::ApplyEffects(const FDialogueEffects& Effects){
	return;   // Step 7; no-op for now
 }