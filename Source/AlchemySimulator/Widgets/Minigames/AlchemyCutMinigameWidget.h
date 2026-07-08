// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AlchemyMinigameWidget.h"
#include "AlchemyCutMinigameWidget.generated.h"


class UHorizontalBox;
class UBorder;

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UAlchemyCutMinigameWidget : public UAlchemyMinigameWidget
{
	GENERATED_BODY()
	

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual void NativeOnFocusLost(const FFocusEvent& InFocusEvent) override;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UHorizontalBox> SegmentContainer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutting Minigame")
    int32 SegmentCount = 20;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutting Minigame")
    int32 GreenSegmentCount = 4;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutting Minigame")
    float MoveInterval = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutting Minigame")
    FLinearColor RedColor = FLinearColor(0.8f, 0.05f, 0.05f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutting Minigame")
    FLinearColor GreenColor = FLinearColor(0.05f, 0.8f, 0.05f, 1.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cutting Minigame")
    FLinearColor IndicatorColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

private:
    TArray<TObjectPtr<UBorder>> Segments;

    int32 CurrentIndex = 0;
    int32 GreenStartIndex = 0;
    int32 GreenEndIndex = 0;

    bool bIsRunning = false;
    bool bFinished = false;

    FTimerHandle MoveTimerHandle;

private:
    void BuildSegments();
    void StartMinigame();
    void MoveIndicator();
    void RefreshVisuals();
    void FinishCut();
    bool IsCurrentIndexSuccess() const;
};
