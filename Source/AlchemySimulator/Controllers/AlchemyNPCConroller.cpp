// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/AlchemyNPCConroller.h"
#include "Components/StateTreeAIComponent.h"
#include "StateTreeEvents.h"

AAlchemyNPCConroller::AAlchemyNPCConroller()
{
    StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAIComponent"));

    BrainComponent = StateTreeAIComponent;

    bStartAILogicOnPossess = true;   // otherwise the tree never runs
    bAttachToPawn          = true;   // needed for EnvQueries

}


void AAlchemyNPCConroller::SendBrainEvent(FGameplayTag EventTag)
{
    if (StateTreeAIComponent)
    {
        FStateTreeEvent Ev;
        Ev.Tag = EventTag;
        StateTreeAIComponent->SendStateTreeEvent(Ev);
    }
}