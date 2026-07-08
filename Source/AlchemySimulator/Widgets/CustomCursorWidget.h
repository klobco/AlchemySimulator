// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "CustomCursorWidget.generated.h"

/**
 * 
 */
UCLASS()
class ALCHEMYSIMULATOR_API UCustomCursorWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// Add an Image named "CursorImage" in your widget Blueprint
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CursorImage = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Cursor")
	TObjectPtr<UTexture2D> CursorTexture = nullptr;

	void SetCursorTexture(UTexture2D* Texture);
};
