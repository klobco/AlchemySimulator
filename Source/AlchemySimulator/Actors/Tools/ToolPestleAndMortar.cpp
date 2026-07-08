// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Tools/ToolPestleAndMortar.h"


AToolPestleAndMortar::AToolPestleAndMortar()
{
    MortarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MortarMesh"));
    MortarMesh->SetupAttachment(RootComponent);
}