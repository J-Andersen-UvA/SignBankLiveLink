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



USTRUCT(BlueprintType)
struct FTargetBlendshapeAndWeight
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString KeyBlendshape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Value = 1.0f;
};

USTRUCT(BlueprintType)
struct FArrTargetBlendshapesAndWeigths
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTargetBlendshapeAndWeight> TargetBlendshapes;
};

USTRUCT(BlueprintType)
struct FSourceToTargetBlendshapes : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FArrTargetBlendshapesAndWeigths> SourceBlendshape;
};

UCLASS()
class testCPPworld_API UBlendshapeMapping : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FSourceToTargetBlendshapes BlendshapeMap;
};
