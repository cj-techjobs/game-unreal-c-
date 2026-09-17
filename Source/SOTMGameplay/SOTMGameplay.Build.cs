// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class SOTMGameplay : ModuleRules
{
	public SOTMGameplay(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"SOTMCore",
			"GameplayAbilities",
			"GameplayTasks"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"AIModule",
			"NavigationSystem",
			"EnhancedInput",
			"GameplayTags"
		});
	}
}
