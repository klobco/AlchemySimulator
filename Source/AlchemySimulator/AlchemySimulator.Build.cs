// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class AlchemySimulator : ModuleRules
{
	public AlchemySimulator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"SlateCore",
            "GameplayTags"
        });

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"AlchemySimulator",
			"AlchemySimulator/Variant_Platforming",
			"AlchemySimulator/Variant_Platforming/Animation",
			"AlchemySimulator/Variant_Combat",
			"AlchemySimulator/Variant_Combat/AI",
			"AlchemySimulator/Variant_Combat/Animation",
			"AlchemySimulator/Variant_Combat/Gameplay",
			"AlchemySimulator/Variant_Combat/Interfaces",
			"AlchemySimulator/Variant_Combat/UI",
			"AlchemySimulator/Variant_SideScrolling",
			"AlchemySimulator/Variant_SideScrolling/AI",
			"AlchemySimulator/Variant_SideScrolling/Gameplay",
			"AlchemySimulator/Variant_SideScrolling/Interfaces",
			"AlchemySimulator/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
