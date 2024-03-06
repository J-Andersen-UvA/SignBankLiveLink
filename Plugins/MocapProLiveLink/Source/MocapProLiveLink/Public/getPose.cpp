// Fill out your copyright notice in the Description page of Project Settings.


#include "getPose.h"


void UgetPose::getPose(FSubjectFrameHandle DataResult, const FString poseName, ExecOutputs& Outputs)
{
    FSubjectMetadata Metadata;

    DataResult.GetSubjectMetadata(Metadata);
    TMap<FName,FString> metadata = Metadata.StringMetadata;

    if (metadata.Num() > 0) {

        const FString pose = metadata["poseName"];
        const bool active = metadata["poseActive"].Equals("True");

        if (poseName == pose && active) {
            Outputs = ExecOutputs::PoseActive;
        }
        else {
            Outputs = ExecOutputs::PoseInactive;
        }
    }
    else {
        Outputs = ExecOutputs::PoseInactive;
    }
}