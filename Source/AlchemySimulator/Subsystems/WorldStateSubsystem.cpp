// Fill out your copyright notice in the Description page of Project Settings.


#include "WorldStateSubsystem.h"


 bool UWorldStateSubsystem::MatchesQuery(const FGameplayTagQuery& Q) const{
    return Q.IsEmpty() || Q.Matches(Flags);
 }


 bool UWorldStateSubsystem::HasFlag(FGameplayTag Flag) const{
    return Flags.HasTagExact(Flag);
 }

 void UWorldStateSubsystem::RemoveFlag(FGameplayTag Flag){
    Flags.RemoveTag(Flag);
    OnFlagRemoved.Broadcast(Flag);
 }

 void UWorldStateSubsystem::AddFlag(FGameplayTag Flag){
    Flags.AddTag(Flag);
    OnFlagAdded.Broadcast(Flag);
 }


 void  UWorldStateSubsystem::AddNumber(FGameplayTag Key, float Delta){
    Numbers.FindOrAdd(Key) += Delta;
 }

 void  UWorldStateSubsystem::SetNumber(FGameplayTag Key, float Value){
    Numbers.FindOrAdd(Key) = Value;
 }

 float UWorldStateSubsystem::GetNumber(FGameplayTag Key) const{
    return Numbers.FindRef(Key);
 }