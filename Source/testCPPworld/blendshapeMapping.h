// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

// Define the testCPPworld_API macro for external use
#ifndef testCPPworld_API
#define testCPPworld_API __declspec(dllexport)
#endif

#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "blendshapeMapping.generated.h"


// Define a struct to represent a target blendshape and its weight
USTRUCT(BlueprintType)
struct FTargetBlendshapeAndWeight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString KeyBlendshape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Weight = 1.0f;
};

// Define a struct to represent an array of target blendshapes and their weights
USTRUCT(BlueprintType)
struct FArrTargetBlendshapesAndWeights
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTargetBlendshapeAndWeight> TargetBlendshapes;
};

// Define a struct to represent mapping from source to target blendshapes
USTRUCT(BlueprintType)
struct FSourceToTargetBlendshapes : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FArrTargetBlendshapesAndWeights> SourceBlendshape;
};

// Define a UObject class to hold the blendshape mapping data
UCLASS()
class testCPPworld_API UBlendshapeMapping : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSourceToTargetBlendshapes BlendshapeMap;
};
