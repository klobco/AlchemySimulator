// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/VerticalBox.h"
#include "Components/Dialogue/DialogueRuntimeComponent.h"
#include "Components/TextBlock.h"
#include "Characters/NPCCharacter.h"
#include "Tags/AlchemyGameplayTags.h"
#include "AlchemySimulatorPlayerController.h"
#include "Widgets/Dialogue/DialogueOptionWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "DialogueWidget.h"


UDialogueWidget::UDialogueWidget()
{
    // The game gets no Enhanced Input while this is open, so these are the only
    // way out short of finishing the conversation. E mirrors the interact key.
    ExitKeys = { EKeys::Escape, EKeys::E, EKeys::Gamepad_FaceButton_Right };
}

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

    if (!Runtime)
    {
        return;
    }

    Runtime->OnStepEntered.AddUniqueDynamic(this, &UDialogueWidget::HandleStepEntered);
    Runtime->OnDialogueEnded.AddUniqueDynamic(this, &UDialogueWidget::HandleDialogueEnded);

    // DisplayStep takes keyboard focus itself, so there is no separate grab here.
    DisplayStep(Runtime->GetCurrentStep());
}

void UDialogueWidget::OnClosed_Implementation()
{
    Super::OnClosed_Implementation();

    if (!Runtime)
    {
        return;
    }

    // Unbind BEFORE ending. EndDialogue broadcasts OnDialogueEnded, which would
    // otherwise re-enter HandleDialogueEnded and pop the stack a second time -
    // and PopWidget takes whatever is on top, not necessarily this widget.
    Runtime->OnStepEntered.RemoveDynamic(this, &UDialogueWidget::HandleStepEntered);
    Runtime->OnDialogueEnded.RemoveDynamic(this, &UDialogueWidget::HandleDialogueEnded);

    if (Runtime->IsInDialogue())
    {
        Runtime->EndDialogue();
    }
}

void UDialogueWidget::NativeOnFocusLost(const FFocusEvent& InFocusEvent)
{
    Super::NativeOnFocusLost(InFocusEvent);

    // Take focus back only while the conversation is live. EndDialogue clears
    // IsInDialogue BEFORE it broadcasts, so by the time teardown moves focus
    // around this guard is already false and a dying widget cannot steal it.
    //
    // WARNING: this only works because the option button in WBP_DialogueOption
    // has "Is Focusable" UNCHECKED. After OnMouseButtonDown, Slate focuses the
    // leaf-most focusable widget under the cursor (SlateApplication.cpp, "set
    // it to the leaf-most widget under the mouse"). A focusable option button
    // wins that, we snatch focus back here mid-gesture, and the press never
    // becomes a click - the player has to click every option twice. If options
    // suddenly need double-clicking, that checkbox is the first thing to check.
    if (Runtime && Runtime->IsInDialogue())
    {
        SetFocus();
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

    if (bAwaitingChoice && DialogueOptionsContainer && OptionWidgetClass)
    {
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

    // Re-assert focus on EVERY step, not just on open. ClearOptions above calls
    // ClearChildren, which destroys a focused option button and would otherwise
    // drop keyboard focus to nothing - one of the two ways the keys went dead.
    FSlateApplication::Get().SetKeyboardFocus(TakeWidget(), EFocusCause::SetDirectly);
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

    // Let the console through so ~ still works while iterating in PIE.
    if (Key == EKeys::Tilde)
    {
        return FReply::Unhandled();
    }

    // Checked before the choices branch, so there is always a way out even
    // while options are on screen. EndDialogue only changes state - the widget
    // closes itself when OnDialogueEnded comes back.
    if (ExitKeys.Contains(Key))
    {
        if (Runtime && Runtime->IsInDialogue())
        {
            Runtime->EndDialogue();
        }
        return FReply::Handled();
    }

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
    }
    else if (Key == EKeys::SpaceBar || Key == EKeys::Enter || Key == EKeys::Gamepad_FaceButton_Bottom)
    {
        if (Runtime)
        {
            Runtime->Advance();
        }
        return FReply::Handled();
    }

    // Swallow everything else. This screen is modal, so it owns the keyboard -
    // and an unhandled key bubbles into Slate navigation, where Tab and the
    // arrow keys would move focus onto an option button and strand these
    // handlers exactly the way a mouse click used to.
    return FReply::Handled();
}