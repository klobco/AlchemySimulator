// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefinitions/ToolItemDefinition.h"

#if WITH_EDITOR

#include "DataAssets/DataAssetProcessingMethod.h"
#include "Misc/DataValidation.h"

#define LOCTEXT_NAMESPACE "ToolItemDefinition"

EDataValidationResult UToolItemDefinition::IsDataValid(FDataValidationContext& Context) const
{
	const EDataValidationResult Result = Super::IsDataValid(Context);

	// Every failure below is invisible at runtime: UMinigameManagerComponent::TryStartToolAction
	// skips unusable actions with a bare `continue` and returns false, and the caller then falls
	// back to dragging the target. A dead tool and a tool the target refused look identical.

	if (Actions.IsEmpty())
	{
		Context.AddWarning(LOCTEXT("NoActions",
			"Actions is empty — this tool cannot do anything. Clicking a target with it will only drag the target."));
	}

	TSet<FGameplayTag> SeenTags;

	for (int32 Index = 0; Index < Actions.Num(); ++Index)
	{
		const FToolAction& Action = Actions[Index];

		if (!Action.ActionTag.IsValid())
		{
			Context.AddWarning(FText::Format(LOCTEXT("NoActionTag",
				"Action {0} has no ActionTag, so no target will ever match it."),
				FText::AsNumber(Index)));
		}
		else if (SeenTags.Contains(Action.ActionTag))
		{
			// TryStartToolAction runs the FIRST action the target accepts, so a duplicate tag is
			// unreachable unless the two entries differ in something the target gates on.
			Context.AddWarning(FText::Format(LOCTEXT("DuplicateActionTag",
				"Action {0} repeats ActionTag '{1}'. The earlier entry wins, so this one is unreachable."),
				FText::AsNumber(Index), FText::FromName(Action.ActionTag.GetTagName())));
		}
		else
		{
			SeenTags.Add(Action.ActionTag);
		}

		if (!Action.MinigameWidgetClass)
		{
			Context.AddWarning(FText::Format(LOCTEXT("NoMinigame",
				"Action {0} has no MinigameWidgetClass. TryStartToolAction silently skips such actions, "
				"so this action can never run."),
				FText::AsNumber(Index)));
		}

		if (Action.ProcessingMethod && !Action.ProcessingMethod->ProcessingTag.IsValid())
		{
			Context.AddWarning(FText::Format(LOCTEXT("MethodWithoutTag",
				"Action {0} uses processing method '{1}', which has no ProcessingTag. The item would be marked "
				"processed but carry no Processing.* tag, so the alchemy math cannot see it."),
				FText::AsNumber(Index), FText::FromString(Action.ProcessingMethod->GetName())));
		}
	}

	return Result;
}

#undef LOCTEXT_NAMESPACE

#endif // WITH_EDITOR
