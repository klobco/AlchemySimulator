// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/AlchemyNPCConroller.h"
#include "Components/StateTreeAIComponent.h"

AAlchemyNPCConroller::AAlchemyNPCConroller()
{
    StateTreeAIComponent = CreateDefaultSubobject<UStateTreeAIComponent>(TEXT("StateTreeAIComponent"));

    BrainComponent = StateTreeAIComponent;
}