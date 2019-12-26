// Fill out your copyright notice in the Description page of Project Settings.


#include "MySettingsObject.h"

void UMySettingsObject::SafeToConfigFile()
{
	this->SaveConfig();
}
