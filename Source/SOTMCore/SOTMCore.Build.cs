// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class SOTMCore : ModuleRules
{
	public SOTMCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"GameplayTags",
			"GameplayAbilities"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
