// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class SOTMPresentation : ModuleRules
{
	public SOTMPresentation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"SOTMCore",
			"SOTMGameplay"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
