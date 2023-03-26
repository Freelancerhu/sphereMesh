// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class space_project : ModuleRules
{
	public space_project(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "NavigationSystem", "AIModule", "ProceduralMeshComponent"});
    }
}
