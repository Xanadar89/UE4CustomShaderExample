// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "CustomShaderExample.h"
#include "Modules/ModuleManager.h"
#include "ShaderParameterUtils.h"

class FCustomShaderModule : public IModuleInterface
{

	void StartupModule() override;

	void ShutdownModule() override;
	
};


void FCustomShaderModule::StartupModule()
{
	FString shadersPath = FPaths::Combine(FPaths::ProjectPluginsDir(), TEXT("../Shaders"));
	AddShaderSourceDirectoryMapping("/GameShaders", shadersPath);
}

void FCustomShaderModule::ShutdownModule()
{
}


IMPLEMENT_PRIMARY_GAME_MODULE(FCustomShaderModule, CustomShaderExample, "CustomShaderExample" );


