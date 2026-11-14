// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDetailsRow.h"
#include "Components/TextBlock.h"

void UItemDetailsRow::SetRow(const FText& InName, const FText& InValue)
{
	if (NameText)
	{
		NameText->SetText(InName);
	}
	if (ValueText)
	{
		ValueText->SetText(InValue);
	}
}
