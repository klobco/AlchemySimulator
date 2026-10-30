// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Inventory/InventoryComponent.h"
#include "Actors/Stations/DryingRack.h"

#include "Components/Processing/ProcessingComponent.h"
#include "DataAssets/DataAssetDryingProfile.h"
#include "ItemDefinitions/ItemDefinitionBase.h"
#include "Tags/AlchemyGameplayTags.h"

ADryingRack::ADryingRack()
{
    DryingRackInventory = CreateDefaultSubobject<UInventoryComponent>(TEXT("Drying Rack Inventory"));
    DryingRackInventory->MaxSlots = 4;

    DryingRackInventory->StackSizeLimit = 1;
    DryingRackInventory->CategoryWhitelist.Add(EItemCategory::Herb);
    DryingRackInventory->CategoryWhitelist.Add(EItemCategory::Ingredient);

}

void ADryingRack::BeginPlay()
{
    Super::BeginPlay();

    if (DryingRackInventory)
    {
        DryingRackInventory->OnInventoryChangedType.AddUniqueDynamic(
            this, &ADryingRack::HandleRackInventoryChanged);
    }

    if (UGameTimeSubsystem* Time = GetWorld()->GetSubsystem<UGameTimeSubsystem>())
    {
        // Stamp first, bind second: the first broadcast must measure from now, not from zero.
        LastDryingStamp = Time->CurrentGameTime.ToTotalMinutes();
        Time->OnGameMinuteChanged.AddUniqueDynamic(this, &ADryingRack::HandleGameMinute);
    }
}

void ADryingRack::Interact_Implementation(APawn* By)
{
    OpenStationInventory();
}

UInventoryComponent* ADryingRack::GetStationInventory() const
{
    return DryingRackInventory;
}

void ADryingRack::HandleRackInventoryChanged(EInventoryChangeType ChangeType, int32 SlotIndex)
{
    // An item that arrives already dried accumulates from the next broadcast on -- the stamp is
    // rack-global, and that is correct. The only job left here is the sanity case: an instance
    // carrying more drying than its profile allows, e.g. after the curve was edited.
    if (ChangeType != EInventoryChangeType::ItemAdded) return;
    if (!DryingRackInventory) return;

    if (SlotIndex == INDEX_NONE)
    {
        for (int32 i = 0; i < DryingRackInventory->Slots.Num(); ++i)
        {
            ClampDryingToProfile(i);
        }
        return;
    }

    ClampDryingToProfile(SlotIndex);
}

void ADryingRack::HandleGameMinute(const FAlchemyGameTime& NewTime)
{
    if (!DryingRackInventory) return;

    // Absolute stamps, not a broadcast count: OnGameMinuteChanged fires once per AddMinutes()
    // call, so a night's sleep arrives as a single broadcast carrying 480 minutes.
    const int64 Now = NewTime.ToTotalMinutes();
    const int64 Delta = Now - LastDryingStamp;
    LastDryingStamp = Now;

    if (Delta <= 0) return;

    const float Elapsed = static_cast<float>(Delta) * DryingRateMultiplier;
    if (Elapsed <= 0.f) return;

    for (int32 i = 0; i < DryingRackInventory->Slots.Num(); ++i)
    {
        if (!DryingRackInventory->Slots[i].IsEmpty())
        {
            AdvanceDrying(i, Elapsed);
        }
    }
}

void ADryingRack::AdvanceDrying(int32 SlotIndex, float ElapsedMinutes)
{
    if (!DryingRackInventory || !DryingRackInventory->Slots.IsValidIndex(SlotIndex)) return;

    // Copies, not references into Slots: UpdateSlotInstance below writes to that array.
    const UItemDefinitionBase* Def = DryingRackInventory->Slots[SlotIndex].Item;
    const FItemInstanceData Original = DryingRackInventory->Slots[SlotIndex].Instance;

    int32 OldStage = INDEX_NONE;
    float ProgressUnused = 0.f;
    if (!UDataAssetDryingProfile::ResolveStageFor(Original, Def, OldStage, ProgressUnused))
    {
        return; // No profile, no duration: this item does not dry.
    }

    // The rack is a processing route like a tool is, so it asks the same gate. A part that may
    // not be dried cannot be dried here either.
    if (!Def->AcceptsProcessingTag(TAG_Processing_Dried)) return;

    // ResolveStageFor only succeeds with a profile and a positive duration behind it.
    const UDataAssetDryingProfile* Profile = Def->DryingProfile;

    FItemInstanceData Inst = Original;

    // Clamping at the last threshold stops DryingMinutes growing forever, and makes two
    // fully-scorched items converge on one value so they can stack again.
    const float Cap = Profile->GetMaxNormalized() * Def->MinutesToFullyDry;
    Inst.DryingMinutes = FMath::Min(Inst.DryingMinutes + ElapsedMinutes, Cap);

    // Already at the cap: nothing changed, so do not write and do not broadcast. Without this a
    // fully scorched item would report ItemUpdated every game minute for the rest of the session.
    if (FMath::IsNearlyEqual(Inst.DryingMinutes, Original.DryingMinutes))
    {
        return;
    }

    int32 NewStage = INDEX_NONE;
    if (!UDataAssetDryingProfile::ResolveStageFor(Inst, Def, NewStage, ProgressUnused)) return;

    if (NewStage != OldStage)
    {
        // Drying is ONE state, not a stack of them: an over-dried item is scorched, not
        // dried-and-scorched. So of all the bands crossed by this tick, only the last one that
        // commits anything is kept -- scan first, commit once.
        UDataAssetProcessingMethod* MethodToCommit = nullptr;
        for (int32 Stage = OldStage + 1; Stage <= NewStage; ++Stage)
        {
            if (!Profile->Stages.IsValidIndex(Stage)) break;

            if (UDataAssetProcessingMethod* Method = Profile->Stages[Stage].CommitMethod)
            {
                MethodToCommit = Method;
            }
        }

        if (MethodToCommit)
        {
            // Replace, don't add. Anything this profile committed on an earlier tick is undone
            // first -- tag, method entry and its potency multiplier together -- so a slow dry
            // and one big time skip land on exactly the same instance.
            for (const FDryingStage& Stage : Profile->Stages)
            {
                if (Stage.CommitMethod && Stage.CommitMethod != MethodToCommit)
                {
                    Inst = UProcessingComponent::RemoveProcessingMethod(Inst, Stage.CommitMethod);
                }
            }

            // Drying is unattended, so there is no minigame quality to fold in.
            Inst = UProcessingComponent::BuildProcessedInstance(Inst, MethodToCommit, 1.0f);
        }

        OnDryingStageChanged.Broadcast(SlotIndex, Profile->Stages[NewStage].StageTag);
    }

    DryingRackInventory->UpdateSlotInstance(SlotIndex, Inst);
}

void ADryingRack::ClampDryingToProfile(int32 SlotIndex)
{
    if (!DryingRackInventory || !DryingRackInventory->Slots.IsValidIndex(SlotIndex)) return;

    const UItemDefinitionBase* Def = DryingRackInventory->Slots[SlotIndex].Item;
    const FItemInstanceData Original = DryingRackInventory->Slots[SlotIndex].Instance;

    int32 Stage = INDEX_NONE;
    float ProgressUnused = 0.f;
    if (!UDataAssetDryingProfile::ResolveStageFor(Original, Def, Stage, ProgressUnused)) return;

    const float Cap = Def->DryingProfile->GetMaxNormalized() * Def->MinutesToFullyDry;
    if (Original.DryingMinutes <= Cap) return;

    FItemInstanceData Inst = Original;
    Inst.DryingMinutes = Cap;
    DryingRackInventory->UpdateSlotInstance(SlotIndex, Inst);
}
