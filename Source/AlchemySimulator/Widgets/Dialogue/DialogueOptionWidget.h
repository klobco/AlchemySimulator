// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Dialogue/AlchemyDialogueTypes.h"
#include "DialogueOptionWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueButton, int32, OutboundIndex);

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UDialogueOptionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UButton* DialogueOptionButton;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DialogueOptionText;
	
	int32 Index;

	UFUNCTION()
	void Setup(int32 Index, const FDialogueOption& Option);

	UPROPERTY(BlueprintReadOnly, Category = "Dialogue")
	FDialogueOption CurrentOption;

	UPROPERTY(BlueprintAssignable, Category = "action")
	FOnDialogueButton DialogueOptionClicked;

	UFUNCTION()
	void OnDialogueOptionClicked();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY()
	TObjectPtr<class UDialogueWidget> Owner = nullptr;
};
