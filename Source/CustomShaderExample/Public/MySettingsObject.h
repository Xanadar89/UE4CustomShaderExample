// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MySettingsObject.generated.h"

/**
 * 
 */
UCLASS(Config=Game, BlueprintType)
class CUSTOMSHADEREXAMPLE_API UMySettingsObject : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, BlueprintReadWrite)
	float MyParam = 10;


	UFUNCTION(BlueprintCallable)
		void SafeToConfigFile();

};
