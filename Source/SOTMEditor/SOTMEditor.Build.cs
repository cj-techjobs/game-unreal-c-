// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class SOTMEditor : ModuleRules
{
	public SOTMEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"EditorSubsystem"
		});
	}
}
