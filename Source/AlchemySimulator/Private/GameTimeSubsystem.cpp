// Fill out your copyright notice in the Description page of Project Settings.


#include "GameTimeSubsystem.h"

void UGameTimeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CurrentGameTime = FAlchemyGameTime();

    CurrentGameTime.Year = 1;
    CurrentGameTime.Month = 1;
    CurrentGameTime.Day = 1;
    CurrentGameTime.Hour = 6;
    CurrentGameTime.Minute = 0;

    AccumulatedTime = 0.0f;

    bTimeIsPaused = false;
}

void UGameTimeSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UGameTimeSubsystem::Tick(float DeltaTime)
{
    if (bTimeIsPaused)
    {
        return;
    }

    
    if (RealSecondsPerGameMinute <= 0.0f)
    {
        return;
    }
    
    AccumulatedTime += DeltaTime;
    
    while (AccumulatedTime >= RealSecondsPerGameMinute)
    {
        AccumulatedTime -= RealSecondsPerGameMinute;
        AddMinutes(1);
    }

}

void UGameTimeSubsystem::RecalculateTime()
{
    const int32 MinutesInHour = 60;
    const int32 HoursInDay = 24;
    const int32 DaysInMonth = 30;
    const int32 MonthsInYear = 12;

    if (CurrentGameTime.Minute >= MinutesInHour)
    {
        CurrentGameTime.Hour += CurrentGameTime.Minute / MinutesInHour;
        CurrentGameTime.Minute %= MinutesInHour;
    }

    if (CurrentGameTime.Hour >= HoursInDay)
    {
        CurrentGameTime.Day += CurrentGameTime.Hour / HoursInDay;
        CurrentGameTime.Hour %= HoursInDay;
    }

    if (CurrentGameTime.Day > DaysInMonth)
    {
        const int32 ZeroIndexedDay = CurrentGameTime.Day - 1;
        CurrentGameTime.Month += ZeroIndexedDay / DaysInMonth;
        CurrentGameTime.Day = (ZeroIndexedDay % DaysInMonth) + 1;
    }

    if (CurrentGameTime.Month > MonthsInYear)
    {
        const int32 ZeroIndexedMonth = CurrentGameTime.Month - 1;
        CurrentGameTime.Year += ZeroIndexedMonth / MonthsInYear;
        CurrentGameTime.Month = (ZeroIndexedMonth % MonthsInYear) + 1;
    }
}


void UGameTimeSubsystem::AddMinutes(int32 MinutesToAdd)
{

    if (MinutesToAdd <= 0)
    {
        return;
    }

    const int32 OldMonth = CurrentGameTime.Month;
    const int32 OldDay = CurrentGameTime.Day;
    const int32 OldHour = CurrentGameTime.Hour;

    CurrentGameTime.Minute += MinutesToAdd;
    RecalculateTime();

    UE_LOG(LogTemp, Log, TEXT("Game Time Updated: %s"), *CurrentGameTime.ToString());

    OnGameMinuteChanged.Broadcast(CurrentGameTime);

    if (CurrentGameTime.Hour != OldHour)
    {
        OnGameHourChanged.Broadcast(CurrentGameTime);
    }
    if (CurrentGameTime.Day != OldDay)
    {
        OnGameDayChanged.Broadcast(CurrentGameTime);
    }
}

float UGameTimeSubsystem::GetNormalizedDay() const
{

    const int32 MinutesToday = (CurrentGameTime.Hour * 60) + CurrentGameTime.Minute;

    return static_cast<float>(MinutesToday) / (24.0f * 60.0f);
}