#include "AlchemyGameplayTags.h"

// Tag strings must match what is serialized in the data assets byte-for-byte — a .uasset stores a
// tag reference by name, so renaming one here silently orphans every asset that used it.

// --- Dialogue ----------------------------------------------------------------------------------
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Dialogue,                 "Dialogue",         "Dialogue Tags");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Dialogue_Speaker,         "Dialogue.Speaker", "Who is talking");
UE_DEFINE_GAMEPLAY_TAG(TAG_Dialogue_Event_Start, "Dialogue.Event.Start");
UE_DEFINE_GAMEPLAY_TAG(TAG_Dialogue_Event_End,   "Dialogue.Event.End");

UE_DEFINE_GAMEPLAY_TAG(TAG_Speaker_Player,       "Speaker.Player");
UE_DEFINE_GAMEPLAY_TAG(TAG_Speaker_NPC,          "Speaker.NPC");

// --- Disease -----------------------------------------------------------------------------------
UE_DEFINE_GAMEPLAY_TAG(TAG_Disease_Fever,        "Disease.Fever");
UE_DEFINE_GAMEPLAY_TAG(TAG_Disease_Infection,    "Disease.Infection");
UE_DEFINE_GAMEPLAY_TAG(TAG_Disease_Nerve,        "Disease.Nerve");
UE_DEFINE_GAMEPLAY_TAG(TAG_Disease_Poison,       "Disease.Poison");

// --- Effects -----------------------------------------------------------------------------------
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect,                   "Effect");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_CalmNerves,        "Effect.CalmNerves");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_FightInfection,    "Effect.FightInfection");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_NeutralizePoison,  "Effect.NeutralizePoison");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_ReduceFever,       "Effect.ReduceFever");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_ReducePain,        "Effect.ReducePain");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_RestoreEnergy,     "Effect.RestoreEnergy");

UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_SideEffect_Drowsiness,   "Effect.SideEffect.Drowsiness");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_SideEffect_IncreaseFever, "Effect.SideEffect.IncreaseFever");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_SideEffect_Nausea,       "Effect.SideEffect.Nausea");
UE_DEFINE_GAMEPLAY_TAG(TAG_Effect_SideEffect_Toxicity,     "Effect.SideEffect.Toxicity");

// --- Nutrients ---------------------------------------------------------------------------------
UE_DEFINE_GAMEPLAY_TAG(TAG_Nutrient_Iron,        "Nutrient.Iron");
UE_DEFINE_GAMEPLAY_TAG(TAG_Nutrient_Minerals,    "Nutrient.Minerals");
UE_DEFINE_GAMEPLAY_TAG(TAG_Nutrient_VitaminA,    "Nutrient.VitaminA");
UE_DEFINE_GAMEPLAY_TAG(TAG_Nutrient_VitaminB,    "Nutrient.VitaminB");
UE_DEFINE_GAMEPLAY_TAG(TAG_Nutrient_VitaminC,    "Nutrient.VitaminC");
UE_DEFINE_GAMEPLAY_TAG(TAG_Nutrient_VitaminD,    "Nutrient.VitaminD");

// --- Plant parts -------------------------------------------------------------------------------
UE_DEFINE_GAMEPLAY_TAG(TAG_PlantPart_Flower,     "PlantPart.Flower");
UE_DEFINE_GAMEPLAY_TAG(TAG_PlantPart_Fruit,      "PlantPart.Fruit");
UE_DEFINE_GAMEPLAY_TAG(TAG_PlantPart_Leaf,       "PlantPart.Leaf");
UE_DEFINE_GAMEPLAY_TAG(TAG_PlantPart_Root,       "PlantPart.Root");
UE_DEFINE_GAMEPLAY_TAG(TAG_PlantPart_Seed,       "PlantPart.Seed");
UE_DEFINE_GAMEPLAY_TAG(TAG_PlantPart_Stem,       "PlantPart.Stem");

// --- Processing: the resulting state, past participle (see "ToolAction.* vs Processing.*") ------
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Processing, "Processing", "Root of the processing-state family. Written onto FItemInstanceData::ProcessingTags");
UE_DEFINE_GAMEPLAY_TAG(TAG_Processing_Boiled,    "Processing.Boiled");
UE_DEFINE_GAMEPLAY_TAG(TAG_Processing_Crushed,   "Processing.Crushed");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_Processing_Cut, "Processing.Cut", "Cut");
UE_DEFINE_GAMEPLAY_TAG(TAG_Processing_Dried,     "Processing.Dried");
UE_DEFINE_GAMEPLAY_TAG(TAG_Processing_Raw,       "Processing.Raw");

// --- Substances --------------------------------------------------------------------------------
UE_DEFINE_GAMEPLAY_TAG(TAG_Substance_Frostaline, "Substance.Frostaline");

// --- Symptoms ----------------------------------------------------------------------------------
UE_DEFINE_GAMEPLAY_TAG(TAG_Symptom_Fever,        "Symptom.Fever");
UE_DEFINE_GAMEPLAY_TAG(TAG_Symptom_Headache,     "Symptom.Headache");
UE_DEFINE_GAMEPLAY_TAG(TAG_Symptom_Nausea,       "Symptom.Nausea");
UE_DEFINE_GAMEPLAY_TAG(TAG_Symptom_Pain,         "Symptom.Pain");
UE_DEFINE_GAMEPLAY_TAG(TAG_Symptom_Sweating,     "Symptom.Sweating");
UE_DEFINE_GAMEPLAY_TAG(TAG_Symptom_Weakness,     "Symptom.Weakness");

// --- Tool actions: the verb, imperative (see "ToolAction.* vs Processing.*") --------------------
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ToolAction, "ToolAction", "Root of the tool-action family. Gates which tool reaches which target");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ToolAction_Crush, "ToolAction.Crush", "Grind a plant part with a pestle and mortar");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_ToolAction_Cut,   "ToolAction.Cut",   "Cut a part off a whole plant with a knife");

// --- World state -------------------------------------------------------------------------------
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_WorldState,      "WorldState",      "World state source of truth");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_WorldState_Flag, "WorldState.Flag", "The different flags");
UE_DEFINE_GAMEPLAY_TAG_COMMENT(TAG_WorldState_NPC,  "WorldState.NPC",  "The NPC information");
