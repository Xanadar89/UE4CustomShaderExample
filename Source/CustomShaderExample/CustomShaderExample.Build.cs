// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CustomShaderExample : ModuleRules
{
	public CustomShaderExample(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core",
															"CoreUObject",
															"Engine",
															"InputCore",
															"HeadMountedDisplay",
															"RenderCore",
															"RHI" });
	}
}
