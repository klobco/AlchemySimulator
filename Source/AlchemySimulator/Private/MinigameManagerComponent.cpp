// Fill out your copyright notice in the Description page of Project Settings.


#include "MinigameManagerComponent.h"
#include "Blueprint/UserWidget.h"

// Sets default values for this component's properties
UMinigameManagerComponent::UMinigameManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UMinigameManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMinigameManagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UMinigameManagerComponent::StartMinigame(TSubclassOf<UUserWidget> MinigameWidgetClass)
{
    if (!MinigameWidgetClass) return;

    APlayerController* PC = Cast<APlayerController>(GetOwner());
    if (!PC) return;

    ActiveMinigameWidget = CreateWidget<UUserWidget>(PC, MinigameWidgetClass);

    if (ActiveMinigameWidget)
    {
        ActiveMinigameWidget->AddToViewport();

        PC->SetInputMode(FInputModeUIOnly());
        PC->bShowMouseCursor = true;
    }
}

void UMinigameManagerComponent::StopMinigame()
{
	if (ActiveMinigameWidget)
	{
		ActiveMinigameWidget->RemoveFromParent();
		ActiveMinigameWidget = nullptr;

		APlayerController* PC = Cast<APlayerController>(GetOwner());
		if (PC)
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
	}
}