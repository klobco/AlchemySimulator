// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameTimeSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FAlchemyGameTime
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 Year = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 Month = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 Day = 1;

	UPROPERTY(BlueprintReadOnly)
	int32 Hour = 0;

	UPROPERTY(BlueprintReadOnly)
	int32 Minute = 0;

	FString ToString() const
	{
		return FString::Printf(TEXT("%04d-%02d-%02d %02d:%02d"), Year, Month, Day, Hour, Minute);
	}

};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameMinuteChanged, const FAlchemyGameTime&, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameHourChanged, const FAlchemyGameTime&, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGameDayChanged, const FAlchemyGameTime&, NewTime);

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UGameTimeSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
	

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override { return true; }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UGameTimeSubsystem, STATGROUP_Tickables); }

	UPROPERTY(BlueprintReadOnly)
	FAlchemyGameTime CurrentGameTime;

	UFUNCTION(BlueprintCallable)
	void AddMinutes(int32 MinutesToAdd);

public:
    UPROPERTY(BlueprintAssignable)
    FOnGameMinuteChanged OnGameMinuteChanged;

    UPROPERTY(BlueprintAssignable)
    FOnGameHourChanged OnGameHourChanged;

    UPROPERTY(BlueprintAssignable)
    FOnGameDayChanged OnGameDayChanged;

	UFUNCTION(BlueprintCallable)
	float GetNormalizedDay() const;


private:

	float RealSecondsPerGameMinute = 0.1f; // 1 real second = 1 game minute

	float AccumulatedTime = 0.0f;

	void AdvanceGameTime();

	void RecalculateTime();

	bool bTimeIsPaused = false;
};
