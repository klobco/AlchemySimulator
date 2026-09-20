// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "WorldStateSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFlagChanged, FGameplayTag, Flag);

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UWorldStateSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable) void AddFlag(FGameplayTag Flag);
    UFUNCTION(BlueprintCallable) void RemoveFlag(FGameplayTag Flag);
    UFUNCTION(BlueprintPure)     bool HasFlag(FGameplayTag Flag) const;
    UFUNCTION(BlueprintPure)     bool MatchesQuery(const FGameplayTagQuery& Q) const;

    UFUNCTION(BlueprintCallable) void  AddNumber(FGameplayTag Key, float Delta);
    UFUNCTION(BlueprintCallable) void  SetNumber(FGameplayTag Key, float Value);
    UFUNCTION(BlueprintPure)     float GetNumber(FGameplayTag Key) const;

    UPROPERTY(BlueprintAssignable) FOnFlagChanged OnFlagAdded;
    UPROPERTY(BlueprintAssignable) FOnFlagChanged OnFlagRemoved;

private:
    UPROPERTY() FGameplayTagContainer Flags;
    UPROPERTY() TMap<FGameplayTag, float> Numbers;
};
