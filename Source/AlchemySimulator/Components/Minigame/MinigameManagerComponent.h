// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Widgets/Minigames/AlchemyMinigameWidget.h"
#include "ItemMetadata.h"
#include "MinigameManagerComponent.generated.h"

class UToolItemDefinition;
class UPrimitiveComponent;

/**
 * The tool action waiting on the active minigame's result.
 *
 * Held as weak pointers so a target destroyed mid-minigame simply resolves to nothing, and
 * cleared by StopMinigame() so a cancelled minigame can never resolve later. This replaces the
 * old pattern where each target AddDynamic'd itself to the manager's delegate — those bindings
 * leaked whenever a minigame was cancelled instead of completed.
 */
USTRUCT()
struct FPendingToolAction
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<AActor> Target;

	UPROPERTY()
	TWeakObjectPtr<UToolItemDefinition> Tool;

	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> HitComponent;

	/** Stored by value, not by index — the tool's Actions array may change while the minigame runs. */
	UPROPERTY()
	FToolAction Action;

	void Reset()
	{
		Target.Reset();
		Tool.Reset();
		HitComponent.Reset();
		Action = FToolAction();
	}
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ALCHEMYSIMULATOR_API UMinigameManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMinigameManagerComponent();

protected:
	virtual void BeginPlay() override;

public:
    UPROPERTY(BlueprintAssignable)
    FOnMinigameFinished OnMinigameFinished;

    /** Full result, including Score and QualityMultiplier. Prefer this over OnMinigameFinished. */
    UPROPERTY(BlueprintAssignable)
    FOnMinigameResultFinished OnMinigameResultFinished;

    /**
     * Run the first action on Tool that Target accepts, then apply its result to Target.
     *
     * Returns false when the tool has nothing to do here (wrong tool, target refuses, no
     * minigame configured) so the caller can fall through to its default click behaviour.
     */
    UFUNCTION(BlueprintCallable, Category = "Minigame")
    bool TryStartToolAction(UToolItemDefinition* Tool, AActor* Target, UPrimitiveComponent* HitComponent);

    UFUNCTION(BlueprintCallable)
    void StartMinigame(TSubclassOf<UAlchemyMinigameWidget> MinigameWidgetClass);

	UFUNCTION(BlueprintCallable)
    void StopMinigame();

    /** The minigame currently on screen, or null. Drives input-mode priority. */
    UFUNCTION(BlueprintPure)
    UAlchemyMinigameWidget* GetActiveMinigameWidget() const { return ActiveMinigameWidget; }

private:
    UPROPERTY()
    TObjectPtr<UAlchemyMinigameWidget> ActiveMinigameWidget;

    UPROPERTY()
    FPendingToolAction PendingAction;

    UFUNCTION()
    void HandleMinigameResult(FMinigameResult Result);

};
