// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "blendshapeMapping.h"
#include "LLFDataAndMapToOutput.generated.h"

/**
 * 
 */
UCLASS()
class TESTCPPWORLD_API ULLFDataAndMapToOutput : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Live Link Functionalities")
	// static TMap<FString, int32> ProcessTable(const TMap<FString, int32>& InputTable);
	static TMap<FString, int32> ProcessTable(const FSourceToTargetBlendshapes& InputTable);
};
