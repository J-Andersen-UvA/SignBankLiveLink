// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.
#include "MocapProLiveLinkSourceFactory.h"
#include "MocapProLiveLinkSource.h"
#include "SMocapProLiveLinkSourceFactory.h"

#define LOCTEXT_NAMESPACE "JSONLiveLinkSourceFactory"

FText UMocapProLiveLinkSourceFactory::GetSourceDisplayName() const
{
	//Returns a localized string for the source name
	return LOCTEXT("SourceDisplayName", "Mocap Pro Glove");
}

FText UMocapProLiveLinkSourceFactory::GetSourceTooltip() const
{
	// Returns a localized string for the source UI tooltip
	return LOCTEXT("SourceTooltip", "Creates a connection to a Mocap Pro Hand Engine");
}

TSharedPtr<SWidget> UMocapProLiveLinkSourceFactory::BuildCreationPanel(FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const
{
	return SNew(SMocapProLiveLinkSourceFactory)
		.OnOkClicked(SMocapProLiveLinkSourceFactory::FOnOkClicked::CreateUObject(this, &UMocapProLiveLinkSourceFactory::OnOkClicked, InOnLiveLinkSourceCreated));
}

TSharedPtr<ILiveLinkSource> UMocapProLiveLinkSourceFactory::CreateSource(const FString& InConnectionString) const
{
	FIPv4Endpoint DeviceEndPoint;
	if (!FIPv4Endpoint::Parse(InConnectionString, DeviceEndPoint))
	{
		return TSharedPtr<ILiveLinkSource>();
	}

	return MakeShared<FMocapProLiveLinkSource>(DeviceEndPoint);
}

void UMocapProLiveLinkSourceFactory::OnOkClicked(FIPv4Endpoint InEndpoint, FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const
{
	InOnLiveLinkSourceCreated.ExecuteIfBound(MakeShared<FMocapProLiveLinkSource>(InEndpoint), InEndpoint.ToString());
}

#undef LOCTEXT_NAMESPACE