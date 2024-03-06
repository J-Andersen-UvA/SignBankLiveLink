// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "MocapProLiveLinkSource.h"

#include "ILiveLinkClient.h"
#include "LiveLinkTypes.h"
#include "Roles/LiveLinkAnimationRole.h"
#include "Roles/LiveLinkAnimationTypes.h"

#include "Async/Async.h"
#include "Common/TcpSocketBuilder.h"
#include "HAL/RunnableThread.h"
#include "Json.h"
#include "Sockets.h"
#include "SocketSubsystem.h"

#define LOCTEXT_NAMESPACE "MocapProLiveLinkSource"

#define RECV_BUFFER_SIZE 1024 * 1024

FMocapProLiveLinkSource::FMocapProLiveLinkSource(FIPv4Endpoint InEndpoint)
	: Socket(nullptr)
	, Stopping(false)
	, Thread(nullptr)
	, WaitTime(FTimespan::FromMilliseconds(10))
{
	// defaults
	DeviceEndpoint = InEndpoint;

	FString ipAdd = (DeviceEndpoint.ToInternetAddr().Get().ToString(true));

	SourceStatus = LOCTEXT("SourceStatus_DeviceNotFound", "Device Not Found");
	SourceType = LOCTEXT("MocapProLiveLinkSourceType", "Mocap Pro LiveLink");
	SourceMachineName = FText::FromString(ipAdd);// FText::Format(LOCTEXT("MocapProLiveLinkSourceMachineName", FText::FromString(ipAdd));

	//SourceMachineName = LOCTEXT("MocapProLiveLinkSourceMachineName", "{ipAdd}");

	//setup socket

	Socket = FTcpSocketBuilder(TEXT("MOCAPPROSOCKET"))
		.WithSendBufferSize(RECV_BUFFER_SIZE)
		.WithReceiveBufferSize(RECV_BUFFER_SIZE)
		;


	Socket->Connect(DeviceEndpoint.ToInternetAddr().Get());

	//Pause thread to allow connnection state to update (This should be changed in the future)
	FPlatformProcess::Sleep(0.1);

	if ((Socket != nullptr) && (Socket->GetSocketType() == SOCKTYPE_Streaming) && Socket->GetConnectionState() == SCS_Connected)
	{
		SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

		Start();

		SourceStatus = LOCTEXT("SourceStatus_Receiving", "Receiving");
	}
}

FMocapProLiveLinkSource::~FMocapProLiveLinkSource()
{
	Stop();
	
	if (Thread)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}
	if (Socket)
	{
		Socket->Shutdown(ESocketShutdownMode::ReadWrite);
		Socket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Socket);
		Socket = nullptr;
	}
	
}

void FMocapProLiveLinkSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	Client = InClient;
	SourceGuid = InSourceGuid;
}


bool FMocapProLiveLinkSource::IsSourceStillValid() const
{
	// Source is valid if we have a valid thread and socket
	bool bIsSourceValid = !Stopping && Thread != nullptr && Socket != nullptr;
	return bIsSourceValid;
}


bool FMocapProLiveLinkSource::RequestSourceShutdown()
{

	return true;
}
// FRunnable interface

void FMocapProLiveLinkSource::Start()
{
	ThreadName = "Mocap Pro TCP Receiver ";
	ThreadName.AppendInt(FAsyncThreadIndex::GetNext());

	Thread = FRunnableThread::Create(this, *ThreadName, 128 * 1024, TPri_AboveNormal, FPlatformAffinity::GetPoolThreadMask());
}

void FMocapProLiveLinkSource::Stop()
{
	Stopping = true;
}

uint32 FMocapProLiveLinkSource::Run()
{
	TSharedRef<FInternetAddr> Sender = SocketSubsystem->CreateInternetAddr();

	long index = 0;
	long Target = 0;
	int32 packet = 0;
	while (!Stopping)
	{
	
			if (Socket->Wait(ESocketWaitConditions::WaitForRead, WaitTime))
			{
				
				if (Socket)
				{				
					uint32 PendingDataSize = 0;
					while (Socket->HasPendingData(PendingDataSize) && PendingDataSize > 0 && !Stopping)
					{
						TArray<uint8, TInlineAllocator<256>> ReceivedSizeData;
						ReceivedSizeData.SetNumZeroed(PendingDataSize + 1);
						int32 BytesRead = 0;
						if (Socket->Recv(ReceivedSizeData.GetData(), 4, BytesRead) && BytesRead > 0)
						{
							
							long Read = ReceivedSizeData.GetData()[0] * 16777216 + ReceivedSizeData.GetData()[1] * 65536 + ReceivedSizeData.GetData()[2] * 256 + ReceivedSizeData.GetData()[3];
							Target = Read;
							
							TArray<uint8, TInlineAllocator<256>> ReceivedData;
							ReceivedData.SetNumZeroed(Target);
							BytesRead = 0;
							int32 BytesReadSum = 0;
							while (BytesReadSum != Target && !Stopping) {
								if (Socket->HasPendingData(PendingDataSize) && PendingDataSize > 0) {
									int32 BytesToRead = Target - BytesReadSum;
									
									if (Socket->Recv(&(ReceivedData.GetData())[BytesReadSum], BytesToRead, BytesRead) && BytesRead > 0)
									{
										BytesReadSum += BytesRead;
										
									}
								}
							}
							if (!Stopping) {
								TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> mReceivedData = MakeShareable(new TArray<uint8>());
								mReceivedData->SetNumUninitialized(Target);
								memcpy(mReceivedData->GetData(), ReceivedData.GetData(), Target);
								AsyncTask(ENamedThreads::GameThread, [this, mReceivedData]() { HandleReceivedData(mReceivedData); });
							}

						}

					}
				}
			
			}

	}
	return 0;
}

void FMocapProLiveLinkSource::HandleReceivedData(TSharedPtr<TArray<uint8>, ESPMode::ThreadSafe> ReceivedData)
{
		if (!Stopping) {
			//Convert byte array to string
			FString JsonString;
			JsonString.Empty(ReceivedData->Num());
			for (uint8& Byte : *ReceivedData.Get())
			{
				JsonString += TCHAR(Byte);
			}

			TSharedPtr<FJsonObject> JsonObject;
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

			if (FJsonSerializer::Deserialize(Reader, JsonObject))
			{

				//Get name of this hand object
				FString SubjectName;
				JsonObject->TryGetStringField(TEXT("name"), SubjectName);

				//Get name of this actor object
				FString ActorName;
				if (JsonObject->TryGetStringField(TEXT("actor"), ActorName)) {
					SubjectName = SubjectName + ": " + ActorName;
				}

				//Get device ID of this hand object
				int DeviceId;
				if (JsonObject->TryGetNumberField(TEXT("deviceID"), DeviceId)) {
					SubjectName = SubjectName + " (" + FString::FromInt(DeviceId) + ")";
				}

				//Get the handedness of this asset
				int Hand_LR;
				JsonObject->TryGetNumberField(TEXT("side"), Hand_LR);

				//------------ Pose ID------------------//
				//Get the name of snap pose (Classifier or hybrid mode)
				FString PoseName;
				JsonObject->TryGetStringField(TEXT("poseName"), PoseName);

				//Get the active state of pose (Is the snap active)
				bool PoseActive;
				JsonObject->TryGetBoolField(TEXT("poseActive"), PoseActive);

				//Get the ID of selected Pose
				int PoseID;
				JsonObject->TryGetNumberField(TEXT("poseId"), PoseID);

				//Get the ID of selected Pose
				double PoseConf;
				JsonObject->TryGetNumberField(TEXT("poseConf"), PoseConf); 

				//--------------------------------------//

				//Get bone array
				TArray<TSharedPtr<FJsonValue>> BoneArray = JsonObject->GetArrayField("bones");
				BoneArray.RemoveAt(0);	//Remove wrist data


				bool bCreateSubject = !EncounteredSubjects.Contains(FName(*SubjectName));

				if (bCreateSubject)
				{
					FLiveLinkStaticDataStruct StaticDataStruct = FLiveLinkStaticDataStruct(FLiveLinkSkeletonStaticData::StaticStruct());
					FLiveLinkSkeletonStaticData& StaticData = *StaticDataStruct.Cast<FLiveLinkSkeletonStaticData>();

					StaticData.BoneNames.SetNumUninitialized(BoneArray.Num());
					StaticData.BoneParents.SetNumUninitialized(BoneArray.Num());

					for (int BoneIdx = 0; BoneIdx < BoneArray.Num(); BoneIdx++)
					{
						const TSharedPtr<FJsonValue>& Bone = BoneArray[BoneIdx];
						const TSharedPtr<FJsonObject> BoneObject = BoneArray[BoneIdx]->AsObject();

						FString BoneName;
						if (BoneObject->TryGetStringField(TEXT("name"), BoneName))
						{
							StaticData.BoneNames[BoneIdx] = FName(*BoneName);
						}
						else
						{
							// Invalid Json Format
							return;
						}

						int BoneParentIdx;
						if (BoneObject->TryGetNumberField("parent", BoneParentIdx))
						{
							//Decrement parent index by one (wrist has been removed)
							StaticData.BoneParents[BoneIdx] = BoneParentIdx - 1;
						}
						else
						{
							// Invalid Json Format
							return;
						}

					}

					Client->PushSubjectStaticData_AnyThread({ SourceGuid, FName(*SubjectName) }, ULiveLinkAnimationRole::StaticClass(), MoveTemp(StaticDataStruct));
					EncounteredSubjects.Add(FName(*SubjectName));
				}

				FLiveLinkFrameDataStruct FrameDataStruct = FLiveLinkFrameDataStruct(FLiveLinkAnimationFrameData::StaticStruct());
				FLiveLinkAnimationFrameData& FrameData = *FrameDataStruct.Cast<FLiveLinkAnimationFrameData>();
				FLiveLinkMetaData MetaData = FLiveLinkMetaData();
				MetaData.StringMetaData.Add("PoseName", PoseName);					//String
				MetaData.StringMetaData.Add("PoseID", FString::FromInt(PoseID));			//Int to string
				MetaData.StringMetaData.Add("PoseActive", PoseActive ? "True" : "False");	//Boolean to string
				MetaData.StringMetaData.Add("PoseConf", FString::SanitizeFloat(PoseConf));		//Float to string
				FrameData.MetaData = MetaData;
				
				FrameData.Transforms.SetNumUninitialized(BoneArray.Num());
				for (int BoneIdx = 0; BoneIdx < BoneArray.Num(); BoneIdx++)
				{
					const TSharedPtr<FJsonValue>& Bone = BoneArray[BoneIdx];
					const TSharedPtr<FJsonObject> BoneObject = Bone->AsObject();

					const TArray<TSharedPtr<FJsonValue>>* LocationArray;
					FVector BoneLocation;
					if (BoneObject->TryGetArrayField(TEXT("translation"), LocationArray)
						&& LocationArray->Num() == 3) // X, Y, Z
					{
						double X = atof(TCHAR_TO_ANSI(*(*LocationArray)[0]->AsString()));
						double Y = -atof(TCHAR_TO_ANSI(*(*LocationArray)[1]->AsString()));
						double Z = atof(TCHAR_TO_ANSI(*(*LocationArray)[2]->AsString()));

						BoneLocation = FVector(X, Y, Z);
					}
					else
					{
						// Invalid Json Format
						return;
					}

					const TArray<TSharedPtr<FJsonValue>>* RotationArray;
					FQuat BoneQuat;
					if (BoneObject->TryGetArrayField(TEXT("rotation"), RotationArray)
						&& RotationArray->Num() == 4) // X, Y, Z, W
					{
						double X = atof(TCHAR_TO_ANSI(*(*RotationArray)[0]->AsString()));
						double Y = atof(TCHAR_TO_ANSI(*(*RotationArray)[1]->AsString()));
						double Z = atof(TCHAR_TO_ANSI(*(*RotationArray)[2]->AsString()));
						double W = atof(TCHAR_TO_ANSI(*(*RotationArray)[3]->AsString()));
						BoneQuat = FQuat(X, Y, Z, W);
					}
					else
					{
						// Invalid Json Format
						return;
					}

					const TArray<TSharedPtr<FJsonValue>>* PreRotationArray;
					FQuat PreBoneQuat;
					if (BoneObject->TryGetArrayField(TEXT("pre_rotation"), PreRotationArray)
						&& PreRotationArray->Num() == 4) // X, Y, Z, W
					{
						double X = atof(TCHAR_TO_ANSI(*(*PreRotationArray)[0]->AsString()));
						double Y = atof(TCHAR_TO_ANSI(*(*PreRotationArray)[1]->AsString()));
						double Z = atof(TCHAR_TO_ANSI(*(*PreRotationArray)[2]->AsString()));
						double W = atof(TCHAR_TO_ANSI(*(*PreRotationArray)[3]->AsString()));
						PreBoneQuat = FQuat(X, Y, Z, W);
					}
					else
					{
						// Invalid Json Format
						return;
					}


					const TArray<TSharedPtr<FJsonValue>>* PostRotationArray;
					FQuat PostBoneQuat;
					if (BoneObject->TryGetArrayField(TEXT("post_rotation"), PostRotationArray)
						&& PostRotationArray->Num() == 4) // X, Y, Z, W
					{
						double X = atof(TCHAR_TO_ANSI(*(*PostRotationArray)[0]->AsString()));
						double Y = atof(TCHAR_TO_ANSI(*(*PostRotationArray)[1]->AsString()));
						double Z = atof(TCHAR_TO_ANSI(*(*PostRotationArray)[2]->AsString()));
						double W = atof(TCHAR_TO_ANSI(*(*PostRotationArray)[3]->AsString()));
						PostBoneQuat = FQuat(X, Y, Z, W);
					}
					else
					{
						// Invalid Json Format
						return;
					}


					//Combine Rotations
					BoneQuat = PreBoneQuat * BoneQuat * PostBoneQuat;
					BoneQuat.X = -BoneQuat.X;
					BoneQuat.Z = -BoneQuat.Z;

					const TArray<TSharedPtr<FJsonValue>>* ScaleArray;
					FVector BoneScale;
					if (BoneObject->TryGetArrayField(TEXT("scale"), ScaleArray)
						&& ScaleArray->Num() == 3) // X, Y, Z
					{
						double X = atof(TCHAR_TO_ANSI(*(*ScaleArray)[0]->AsString()));
						double Y = atof(TCHAR_TO_ANSI(*(*ScaleArray)[1]->AsString()));
						double Z = atof(TCHAR_TO_ANSI(*(*ScaleArray)[2]->AsString()));
						BoneScale = FVector(X, Y, Z);
					}
					else
					{
						// Invalid Json Format
						return;
					}

					FrameData.Transforms[BoneIdx] = FTransform(BoneQuat, BoneLocation, BoneScale);

				}

				Client->PushSubjectFrameData_AnyThread({ SourceGuid, FName(*SubjectName) }, MoveTemp(FrameDataStruct));
			}
		}
}

#undef LOCTEXT_NAMESPACE
