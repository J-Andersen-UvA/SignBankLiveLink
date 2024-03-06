// Fill out your copyright notice in the Description page of Project Settings.


#include "LLFDataAndMapToOutput.h"
#include "ILiveLinkClient.h"

TMap<FString, int32> ULLFDataAndMapToOutput::ProcessTable(const FSourceToTargetBlendshapes& InputTable)
{
	//ILiveLinkClient
	//GetPropertyValue(UPARAM(ref) FLiveLinkBasicBlueprintData & BasicData, FName PropertyName, float& Value);
	
	TMap<FString, int32> OutputMap;
	return OutputMap;
}