// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomCursorWidget.h"
#include "Components/Image.h"

void UCustomCursorWidget::SetCursorTexture(UTexture2D* Texture)
{
	CursorTexture = Texture;
	if (CursorImage && Texture)
	{
		CursorImage->SetBrushFromTexture(Texture, true);
	}
}

