// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/Menu/BaseGameWidget.h"
#include "Dialogue/AlchemyDialogueTypes.h"
#include "DialogueWidget.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDialogueWidget : public UBaseGameWidget
{
	GENERATED_BODY()
	
public:

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* DialogueOptionsContainer;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SpeakerText = nullptr;

    UPROPERTY(meta = (BindWidget))
	class UTextBlock* BodyText = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FDialogueStep CurrentStep;

	UPROPERTY(EditAnywhere, Category = "Dialogue")
    TSubclassOf<class UDialogueOptionWidget> OptionWidgetClass;

	UFUNCTION()
	void OnDialogueOptionClicked(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void Setup(class UDialogueRuntimeComponent* InRuntime, class ANPCCharacter* InSpeaker);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void OnOpened_Implementation() override;
    virtual void OnClosed_Implementation() override;
    virtual FReply NativeOnKeyDown(const FGeometry& Geo, const FKeyEvent& Event) override;

    UFUNCTION()
	void HandleStepEntered(const FDialogueStep& Step);

    UFUNCTION()
	void HandleDialogueEnded();

    void DisplayStep(const FDialogueStep& Step);
    void ClearOptions();
    FText ResolveSpeakerName(const FDialogueNode& Node) const;

    UPROPERTY()
	TObjectPtr<class UDialogueRuntimeComponent> Runtime = nullptr;

    UPROPERTY()
	TObjectPtr<ANPCCharacter> Speaker = nullptr;

    UPROPERTY()
	TArray<TObjectPtr<class UDialogueOptionWidget>> OptionWidgets;

    bool bAwaitingChoice = false;
};
