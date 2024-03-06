// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Roles/LiveLinkAnimationBlueprintStructs.h"
#include "getPose.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class ExecOutputs : uint8
{
	PoseActive,
	PoseInactive
};


UCLASS()
class MOCAPPROLIVELINK_API UgetPose : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:

		UFUNCTION(BlueprintCallable, meta = (DisplayName = "getPose", Keywords = "getPose get Pose HandEngine Hand Engine", ExpandEnumAsExecs = "Branches"), Category = HandEngine)
		static void getPose(const FSubjectFrameHandle DataResult, const FString PoseName, ExecOutputs& Branches);

};
