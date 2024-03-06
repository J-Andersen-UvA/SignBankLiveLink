// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "SMocapProLiveLinkSourceFactory.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "MocapProLiveLinkSourceEditor"

void SMocapProLiveLinkSourceFactory::Construct(const FArguments& Args)
{
	OkClicked = Args._OnOkClicked;

	//Define default port settings
	FIPv4Endpoint Endpoint;
	Endpoint.Address = FIPv4Address(127, 0, 0, 1);
	Endpoint.Port = 9000;

	FNumberFormattingOptions form = FNumberFormattingOptions();
	form.SetUseGrouping(false);


	ChildSlot
		[
			SNew(SBox)
			.WidthOverride(250)
		[
			SNew(SVerticalBox)
			//Add new slot in vertical stack (This will contain Address Settings)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			//Create horizontal layout
			SNew(SHorizontalBox)
			//Add slot to left for label
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.FillWidth(0.5f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TCPAddress", "Address"))
		]
	//Add slot to right for editText field
	+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.FillWidth(0.5f)
		[
			SAssignNew(EditabledText_Address, SEditableTextBox)
			.Text(FText::FromString(Endpoint.Address.ToString()))
		.OnTextCommitted(this, &SMocapProLiveLinkSourceFactory::OnEndpointAddressChanged)
		]
		]
	//Add new slot in vertical stack (This will contain Port Settings)
	+ SVerticalBox::Slot()
		.AutoHeight()
		[
			//Create horizontal layout
			SNew(SHorizontalBox)
			//Add slot to left for label
		+ SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.FillWidth(0.5f)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("TCPPort", "Port"))
		]
	//Add slot to right for editText field
	+ SHorizontalBox::Slot()
		.HAlign(HAlign_Fill)
		.FillWidth(0.5f)
		[
			SAssignNew(EditabledText_Port, SEditableTextBox)
			.Text(FText::AsNumber(Endpoint.Port, &form))
		.OnTextCommitted(this, &SMocapProLiveLinkSourceFactory::OnEndpointPortChanged)
		]
		]
	//Add new slow in vertical stack (This will contain the confirmation box)
	+ SVerticalBox::Slot()
		.HAlign(HAlign_Right)
		.AutoHeight()
		[
			//Add button
			SNew(SButton)
			.OnClicked(this, &SMocapProLiveLinkSourceFactory::OnOkClicked)
		[
			SNew(STextBlock)
			.Text(LOCTEXT("Ok", "Ok"))
		]
		]
		]
		];
}

void SMocapProLiveLinkSourceFactory::OnEndpointAddressChanged(const FText& NewValue, ETextCommit::Type)
{
	FNumberFormattingOptions form = FNumberFormattingOptions();
	form.SetUseGrouping(false);

	//Address setting updated
	TSharedPtr<SEditableTextBox> EditabledTextPin_Address = EditabledText_Address.Pin();
	TSharedPtr<SEditableTextBox> EditabledTextPin_Port = EditabledText_Port.Pin();
	if (EditabledTextPin_Address.IsValid())
	{
		FIPv4Endpoint Endpoint;
		//Set old port and new address to end point
		if (!FIPv4Endpoint::Parse(NewValue.ToString() + ":" + EditabledTextPin_Port->GetText().ToString(), Endpoint))
		{
			//Settings are invalid, revert
			Endpoint.Address = FIPv4Address(127, 0, 0, 1);	//Default address
			EditabledTextPin_Address->SetText(FText::FromString(Endpoint.Address.ToString()));
		}

	}
}


void SMocapProLiveLinkSourceFactory::OnEndpointPortChanged(const FText& NewValue, ETextCommit::Type)
{
	//Port settings have been updated, check if this is a valid input (Revert to default otherwise)
	TSharedPtr<SEditableTextBox> EditabledTextPin_Address = EditabledText_Address.Pin();
	TSharedPtr<SEditableTextBox> EditabledTextPin_Port = EditabledText_Port.Pin();

	//Check if instance of edit text field is valid
	if (EditabledTextPin_Port.IsValid())
	{
		FIPv4Endpoint Endpoint;

		//Set new port and old address to end point
		if (!FIPv4Endpoint::Parse(EditabledTextPin_Address->GetText().ToString() + ":" + NewValue.ToString(), Endpoint))
		{
			//Settings are invalid, revert
			FNumberFormattingOptions form = FNumberFormattingOptions();
			form.SetUseGrouping(false);	//Format string with no commas (i.e. 1000 vs 1,000)

			Endpoint.Port = 9000;	//Default port number
			EditabledTextPin_Port->SetText(FText::AsNumber(Endpoint.Port, &form));
		}

	}
}

FReply SMocapProLiveLinkSourceFactory::OnOkClicked()
{
	//Confirm TCP Port and Address settings

	//Get instance of edit text field
	TSharedPtr<SEditableTextBox> EditabledTextPin_Address = EditabledText_Address.Pin();
	TSharedPtr<SEditableTextBox> EditabledTextPin_Port = EditabledText_Port.Pin();

	//Get instances of edit text fields are valid
	if (EditabledTextPin_Address.IsValid() && EditabledTextPin_Port.IsValid())
	{
		FIPv4Endpoint Endpoint;
		//Set port and address to end point
		if (FIPv4Endpoint::Parse(EditabledTextPin_Address->GetText().ToString() + ":" + EditabledTextPin_Port->GetText().ToString(), Endpoint))
		{
			//Port created successfully 
			OkClicked.ExecuteIfBound(Endpoint);
		}
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE