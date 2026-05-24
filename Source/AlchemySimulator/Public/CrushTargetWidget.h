// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PestleMortarMinigame.h"
#include "CrushTargetWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCrushTargetFinished, ECrushHitResult, HitResult);

/**
 *
 */
UCLASS()
class ALCHEMYSIMULATOR_API UCrushTargetWidget : public UUserWidget
{
	GENERATED_BODY()

public:

    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable)
    void InitTarget(float InLifetime);

    UPROPERTY(BlueprintAssignable)
    FOnCrushTargetFinished OnTargetFinished;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UImage> ImageOuterCircle;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<class UButton> ButtonHit;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crush Target")
    float Lifetime = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crush Target")
    float StartOuterScale = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crush Target")
    float EndOuterScale = 0.3f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crush Target")
    float PerfectProgress = 0.65f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crush Target")
    float PerfectTolerance = 0.08f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Crush Target")
    float GoodTolerance = 0.18f;

private:
    float ElapsedTime = 0.f;
    bool bIsActive = false;

    UFUNCTION()
    void HandleHitClicked();

    void UpdateVisuals(float Progress);
    void ExpireTarget();
    void EvaluateHit();
};
