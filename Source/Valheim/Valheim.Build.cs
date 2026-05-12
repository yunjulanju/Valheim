// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Valheim : ModuleRules
{
	public Valheim(ReadOnlyTargetRules Target) : base(Target)
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
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Valheim",
			"Valheim/Variant_Platforming",
			"Valheim/Variant_Platforming/Animation",
			"Valheim/Variant_Combat",
			"Valheim/Variant_Combat/AI",
			"Valheim/Variant_Combat/Animation",
			"Valheim/Variant_Combat/Gameplay",
			"Valheim/Variant_Combat/Interfaces",
			"Valheim/Variant_Combat/UI",
			"Valheim/Variant_SideScrolling",
			"Valheim/Variant_SideScrolling/AI",
			"Valheim/Variant_SideScrolling/Gameplay",
			"Valheim/Variant_SideScrolling/Interfaces",
			"Valheim/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
