#pragma once
#include "NativeGameplayTags.h"

// Every gameplay tag this project uses is declared here and defined in AlchemyGameplayTags.cpp.
// Config/DefaultGameplayTags.ini no longer lists them: native tags register during module static
// init, which runs before any asset loads, so data assets resolve them exactly as before.
//
// Reference a tag by its symbol (FNativeGameplayTag converts implicitly to FGameplayTag) rather
// than FGameplayTag::RequestGameplayTag(TEXT("...")) — a typo becomes a compile error instead of a
// silently invalid tag, and there is no per-call lookup through UGameplayTagsManager.
//
// Adding a tag means one line here and one in the .cpp. Tags authored in the editor still land in
// the ini; move them here when C++ needs to name them.

// --- Dialogue ----------------------------------------------------------------------------------
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dialogue);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dialogue_Speaker);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dialogue_Event_Start);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Dialogue_Event_End);

ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Speaker_Player);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Speaker_NPC);

// --- Disease -----------------------------------------------------------------------------------
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Disease_Fever);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Disease_Infection);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Disease_Nerve);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Disease_Poison);

// --- Effects -----------------------------------------------------------------------------------
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_CalmNerves);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_FightInfection);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_NeutralizePoison);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_ReduceFever);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_ReducePain);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_RestoreEnergy);

ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_SideEffect_Drowsiness);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_SideEffect_IncreaseFever);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_SideEffect_Nausea);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Effect_SideEffect_Toxicity);

// --- Nutrients ---------------------------------------------------------------------------------
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Nutrient_Iron);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Nutrient_Minerals);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Nutrient_VitaminA);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Nutrient_VitaminB);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Nutrient_VitaminC);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Nutrient_VitaminD);

// --- Plant parts -------------------------------------------------------------------------------
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PlantPart_Flower);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PlantPart_Fruit);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PlantPart_Leaf);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PlantPart_Root);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PlantPart_Seed);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_PlantPart_Stem);

// --- Processing: the resulting state written onto FItemInstanceData::ProcessingTags -------------
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Processing);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Processing_Boiled);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Processing_Crushed);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Processing_Cut);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Processing_Dried);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Processing_Raw);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Processing_Scorched);

// --- Drying: the DERIVED stage of an item on a rack, never written to ProcessingTags -----------
// A third family, deliberately not under Processing.*: AllowedProcessingTags matches with HasTag,
// so a Processing.Drying.Moist would start matching every part that allows Processing.Drying, and
// UDataAssetPlantPart::IsDataValid would accept a stage tag there as a legitimate processing tag.
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Drying);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Drying_Wet);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Drying_Moist);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Drying_SemiDry);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Drying_Dry);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Drying_Scorched);

// --- Substances --------------------------------------------------------------------------------
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Substance_Frostaline);

// --- Symptoms ----------------------------------------------------------------------------------
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Symptom_Fever);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Symptom_Headache);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Symptom_Nausea);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Symptom_Pain);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Symptom_Sweating);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Symptom_Weakness);

// --- Tool actions -----------
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ToolAction);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ToolAction_Crush);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_ToolAction_Cut);

// --- World state -------------------------------------------------------------------------------
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_WorldState);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_WorldState_Flag);
ALCHEMYSIMULATOR_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_WorldState_NPC);
