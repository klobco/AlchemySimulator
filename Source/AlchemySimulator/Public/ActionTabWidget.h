// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ActionTabWidget.generated.h"


UENUM(BlueprintType)
enum class EAction : uint8
{
	None            UMETA(DisplayName = "None"),
	AddToTable      UMETA(DisplayName = "Add to Table"),
	RemoveFromTable UMETA(DisplayName = "Remove from Table"),
	AddHerb		    UMETA(DisplayName = "Add Herb"),
	Count           UMETA(Hidden)
};

ENUM_RANGE_BY_COUNT(EAction, EAction::Count);


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActionButton,
	EAction, Action);
/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UActionTabWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget)) class UTextBlock* ActionName = nullptr;
	UPROPERTY(meta = (BindWidget)) class UButton* ActionButton = nullptr;

	UPROPERTY(BlueprintAssignable, Category = "action")
	FOnActionButton ActionClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "action")
	EAction Action;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "action")
	FText ActionText;

	UFUNCTION()
	void HandleActionClicked();

	UFUNCTION(BlueprintCallable)
	void Setup(FText Text, EAction Acc);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	void Refresh();

};
