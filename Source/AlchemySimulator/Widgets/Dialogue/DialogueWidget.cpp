// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/VerticalBox.h"
#include "Components/Dialogue/DialogueRuntimeComponent.h"
#include "Components/TextBlock.h"
#include "Characters/NPCCharacter.h"
#include "Tags/AlchemyGameplayTags.h"
#include "AlchemySimulatorPlayerController.h"
#include "Widgets/Dialogue/DialogueOptionWidget.h"
#include "DialogueWidget.h"


void UDialogueWidget::NativeConstruct()
{
    Super::NativeConstruct();
    SetIsFocusable(true);
}

void UDialogueWidget::NativeDestruct()
{
    Super::NativeDestruct();
}

void UDialogueWidget::Setup(UDialogueRuntimeComponent* InRuntime, ANPCCharacter* InSpeaker)
{
    Runtime = InRuntime;
    Speaker = InSpeaker;
}

void UDialogueWidget::OnDialogueOptionClicked(int32 Index)
{
    if (Runtime)
    {
        Runtime->SelectOption(Index);
    }
}

void UDialogueWidget::OnOpened_Implementation()
{
    Super::OnOpened_Implementation();
    
    if (Runtime)
    {
        Runtime->OnStepEntered.AddUniqueDynamic(this, &UDialogueWidget::HandleStepEntered);
        Runtime->OnDialogueEnded.AddUniqueDynamic(this, &UDialogueWidget::HandleDialogueEnded);
    }

    DisplayStep(Runtime->GetCurrentStep());

    SetKeyboardFocus();
}

void UDialogueWidget::OnClosed_Implementation()
{
    Super::OnClosed_Implementation();

    if (Runtime)
    {
        Runtime->OnStepEntered.RemoveDynamic(this, &UDialogueWidget::HandleStepEntered);
        Runtime->OnDialogueEnded.RemoveDynamic(this, &UDialogueWidget::HandleDialogueEnded);
    }
    
    if (Runtime->IsInDialogue())
    {
        Runtime->EndDialogue();
    }
}

void UDialogueWidget::HandleStepEntered(const FDialogueStep& Step)
{
    DisplayStep(Step);
}

void UDialogueWidget::HandleDialogueEnded()
{

    if (AAlchemySimulatorPlayerController* PC = Cast<AAlchemySimulatorPlayerController>(GetOwningPlayer()))
    {
        PC->PopWidget();
    }
}

void UDialogueWidget::DisplayStep(const FDialogueStep& Step)
{
    ClearOptions();

    if (Step.Type == EDialogueStepType::End)
    {
        return;
    }


    if (SpeakerText) SpeakerText->SetText(ResolveSpeakerName(Step.Node));
    if (BodyText)    BodyText->SetText(Step.Node.Line);

    bAwaitingChoice = (Step.Type == EDialogueStepType::Choices);

    if (!bAwaitingChoice || !DialogueOptionsContainer || !OptionWidgetClass) return;

    UE_LOG(LogTemp, Warning, TEXT("Displaying dialogue step with %d options"), Step.Options.Num());

    for (int32 i = 0; i < Step.Options.Num(); i++)
    {
        UDialogueOptionWidget* OptionWidget = CreateWidget<UDialogueOptionWidget>(this, OptionWidgetClass);
        if (OptionWidget)
        {
            DialogueOptionsContainer->AddChild(OptionWidget);
            OptionWidget->Setup(i, Step.Options[i]);
            OptionWidget->DialogueOptionClicked.AddDynamic(this, &UDialogueWidget::OnDialogueOptionClicked);
            OptionWidgets.Add(OptionWidget);
        }
    }
}

void UDialogueWidget::ClearOptions()
{
    if (DialogueOptionsContainer)
    {
        DialogueOptionsContainer->ClearChildren();
    }
    OptionWidgets.Reset();
}

FText UDialogueWidget::ResolveSpeakerName(const FDialogueNode& Node) const
{
    if (Node.SpeakerTag.MatchesTagExact(TAG_Speaker_Player))
    {
        return NSLOCTEXT("Dialogue", "SpeakerYou", "You");
    }
    return Speaker ? Speaker->DisplayName : FText::GetEmpty();
}

FReply UDialogueWidget::NativeOnKeyDown(const FGeometry& Geo, const FKeyEvent& Event)
{
    const FKey Key = Event.GetKey();
    if (bAwaitingChoice)
    {
        static const FKey NumberKeys[] = {
            EKeys::One, EKeys::Two,   EKeys::Three, EKeys::Four, EKeys::Five,
            EKeys::Six, EKeys::Seven, EKeys::Eight, EKeys::Nine
        };

        for (int32 i = 0; i < UE_ARRAY_COUNT(NumberKeys); ++i)
        {
            if (Key == NumberKeys[i] && OptionWidgets.IsValidIndex(i))
            {
                OnDialogueOptionClicked(i);
                return FReply::Handled();
            }
        }
        return FReply::Unhandled();
    }

    if (Key == EKeys::SpaceBar || Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom)
    {
        if (Runtime) Runtime->Advance();
        return FReply::Handled();
    }

    return Super::NativeOnKeyDown(Geo, Event);
}