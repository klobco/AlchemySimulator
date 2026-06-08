// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AlchemyMinigameWidget.h"
#include "DataStructHelpers.h"
#include "PestleMortarMinigame.generated.h"

class UCanvasPanel;
class UCrushTargetWidget;
class UProgressBar;
class UTextBlock;

UENUM(BlueprintType)
enum class ECrushHitResult : uint8
{
    Perfect,
    Good,
    Bad,
    Miss
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPestleMortarFinished, FMinigameResult, Result);

/**
 *
 */
UCLASS()
class ALCHEMYSIMULATOR_API UPestleMortarMinigame : public UAlchemyMinigameWidget
{
	GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UPROPERTY(BlueprintAssignable)
    FOnPestleMortarFinished OnPestleMortarFinished;

protected:
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UCanvasPanel> TargetCanvas;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UProgressBar> GroundnessBar;

    UPROPERTY(meta = (BindWidgetOptional))
    TObjectPtr<UTextBlock> ScoreText;

    UPROPERTY(EditDefaultsOnly, Category = "Pestle Mortar")
    TSubclassOf<UCrushTargetWidget> CrushTargetWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pestle Mortar")
    float MinigameDuration = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pestle Mortar")
    float SpawnInterval = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pestle Mortar")
    float TargetLifetime = 1.4f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pestle Mortar")
    FVector2D TargetSize = FVector2D(160.f, 160.f);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Pestle Mortar")
    float RequiredGroundness = 100.f;

private:
    FTimerHandle SpawnTimerHandle;

    float ElapsedTime = 0.f;
    float CurrentGroundness = 0.f;
    int32 Score = 0;
    int32 PerfectHits = 0;
    int32 Misses = 0;
    bool bMinigameEnded = false;
    bool bPendingFirstSpawn = false;

private:
    void StartMinigame();
    void EndMinigame(bool bSuccess);

    void SpawnCrushTarget();
    FVector2D GetRandomTargetPosition() const;

    UFUNCTION()
    void HandleTargetFinished(ECrushHitResult HitResult);

    void AddGroundness(float Amount);
    void RefreshUI();
};
