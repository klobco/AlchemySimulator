// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAssets/DataAssetPlantPart.h"

#include "DataAssets/DataAssetProcessingMethod.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#define LOCTEXT_NAMESPACE "DataAssetPlantPart"

bool UDataAssetPlantPart::AcceptsToolAction(const FToolAction& Action) const
{
	// HasTag, not HasTagExact: a container holding the parent ToolAction should match every child.
	if (!AcceptedToolActions.HasTag(Action.ActionTag))
	{
		return false;
	}

	// A pure-harvest action carries no processing method (cutting a leaf off a plant yields a raw
	// leaf), so it stops here. Only actions that would actually change the item's processing state
	// are asked the second question.
	if (Action.ProcessingMethod)
	{
		if (!bCanBeProcessed)
		{
			return false;
		}

		if (!AllowedProcessingTags.HasTag(Action.ProcessingMethod->ProcessingTag))
		{
			return false;
		}
	}

	return true;
}

#if WITH_EDITOR

EDataValidationResult UDataAssetPlantPart::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult Result = Super::IsDataValid(Context);

	// Everything below is a warning rather than an error: a half-authored part asset is a normal
	// mid-session state, and blocking the save would be worse than the silent failure this exists
	// to replace. The point is that these traps are all invisible at runtime — a failed gate just
	// makes the tool click fall through to dragging the item, with no log anywhere.

	if (AcceptedToolActions.IsEmpty())
	{
		Context.AddWarning(LOCTEXT("NoToolActions",
			"AcceptedToolActions is empty — this part can never be harvested off a plant, and no tool will work on it."));
	}

	if (bCanBeProcessed && AllowedProcessingTags.IsEmpty())
	{
		Context.AddWarning(LOCTEXT("ProcessableButNoTags",
			"bCanBeProcessed is true but AllowedProcessingTags is empty. An empty container allows NOTHING, "
			"so every processing action will silently refuse. Add the Processing.* tags this part should accept."));
	}

	if (!bCanBeProcessed && !AllowedProcessingTags.IsEmpty())
	{
		Context.AddWarning(LOCTEXT("TagsButNotProcessable",
			"AllowedProcessingTags lists tags but bCanBeProcessed is false, so they are all ignored. "
			"Either enable bCanBeProcessed or clear the tags."));
	}

	// Wrong-family tags: MatchesTag is hierarchy-aware, so this keeps working as new children are
	// added under either root. Catches picking a tag from the wrong branch of the picker.
	const FGameplayTag ToolActionRoot = FGameplayTag::RequestGameplayTag(TEXT("ToolAction"), false);
	if (ToolActionRoot.IsValid())
	{
		for (const FGameplayTag& Tag : AcceptedToolActions)
		{
			if (!Tag.MatchesTag(ToolActionRoot))
			{
				Context.AddWarning(FText::Format(LOCTEXT("BadToolActionTag",
					"AcceptedToolActions contains '{0}', which is not a ToolAction.* tag. It will never match any tool action."),
					FText::FromName(Tag.GetTagName())));
			}
		}
	}

	const FGameplayTag ProcessingRoot = FGameplayTag::RequestGameplayTag(TEXT("Processing"), false);
	if (ProcessingRoot.IsValid())
	{
		for (const FGameplayTag& Tag : AllowedProcessingTags)
		{
			if (!Tag.MatchesTag(ProcessingRoot))
			{
				Context.AddWarning(FText::Format(LOCTEXT("BadProcessingTag",
					"AllowedProcessingTags contains '{0}', which is not a Processing.* tag. It will never match any processing method."),
					FText::FromName(Tag.GetTagName())));
			}
		}
	}

	if (!WorldMesh)
	{
		Context.AddWarning(LOCTEXT("NoWorldMesh",
			"WorldMesh is unset — this part will be invisible as a harvested actor, and on the growing plant "
			"unless every FPlantPartHarvestData row overrides Mesh."));
	}

	return Result;
}

#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
