// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IInteractable.h"
#include "GameFramework/Character.h"
#include "NPCCharacter.generated.h"

UCLASS()
class ALCHEMYSIMULATOR_API ANPCCharacter : public ACharacter, public IInteractable
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ANPCCharacter();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Schedule")
    bool bShouldGoToWork = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UPatientConditionComponent* PatientConditionComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="NPC")
    FText DisplayName;

    /** Whoever is currently talking to us. Null when idle. */
    UPROPERTY(EditAnywhere, Category="Dialogue")
    TObjectPtr<APawn> CurrentInteractor = nullptr;

    /** Releases the conversation lock taken by Interact. No-op unless By holds it. */
    UFUNCTION(BlueprintCallable, Category="Dialogue")
    void EndInteraction(APawn* By);

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void BeginConversation(APawn* With);

	UFUNCTION(BlueprintCallable, Category="Dialogue")
	void EndConversation();

	virtual FText   GetInteractPrompt_Implementation() const override;
	virtual FVector GetInteractWorldLocation_Implementation() const override;
	virtual bool    CanInteract_Implementation(APawn* By) const override;
	virtual void    Interact_Implementation(APawn* By) override;
	virtual void    OnFocStart_Implementation(APawn* By) override;
	virtual void    OnFocEnd_Implementation(APawn* By) override;

};
