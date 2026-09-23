// Fill out your copyright notice in the Description page of Project Settings.

#include "PlayerModes/PlayerModeBase.h"
#include "AlchemySimulatorPlayerController.h"
#include "InputMappingContext.h"
#include "InputAction.h"

namespace
{
	/** Resolve a soft asset list, skipping entries whose asset is missing. */
	template <typename T>
	void ResolveSoftList(const TArray<TSoftObjectPtr<T>>& In, TArray<const T*>& Out)
	{
		for (const TSoftObjectPtr<T>& Soft : In)
		{
			if (Soft.IsNull())
			{
				continue;
			}

			if (const T* Resolved = Soft.LoadSynchronous())
			{
				Out.AddUnique(Resolved);
			}
			else
			{
				// A renamed or deleted asset silently widens what the player can
				// press, so say so rather than failing open quietly.
				UE_LOG(LogTemp, Warning, TEXT("[PlayerMode] could not resolve %s"), *Soft.ToString());
			}
		}
	}
}

void UPlayerModeBase::EnterMode()
{
}

void UPlayerModeBase::ExitMode()
{
}

FPlayerModeInputSpec UPlayerModeBase::GetInputSpec() const
{
	// Default: plain gameplay. Subclasses widen this.
	return FPlayerModeInputSpec();
}

void UPlayerModeBase::GetMappingContexts(TArray<const UInputMappingContext*>& OutContexts) const
{
	ResolveSoftList(MappingContexts, OutContexts);
}

void UPlayerModeBase::GetBlockedActions(bool bUIOpen, TArray<const UInputAction*>& OutActions) const
{
	ResolveSoftList(BlockedActions, OutActions);

	if (bUIOpen)
	{
		ResolveSoftList(BlockedActionsWhileUIOpen, OutActions);
	}
}

UWorld* UPlayerModeBase::GetWorld() const
{
	// The CDO is not in a world; returning one from it breaks editor tooling.
	if (HasAnyFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}

	return OwningController ? OwningController->GetWorld() : nullptr;
}
