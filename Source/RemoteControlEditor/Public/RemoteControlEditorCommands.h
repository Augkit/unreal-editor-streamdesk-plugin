// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "RemoteControlEditorStyle.h"

class FRemoteControlEditorCommands : public TCommands<FRemoteControlEditorCommands>
{
public:

	FRemoteControlEditorCommands()
		: TCommands<FRemoteControlEditorCommands>(TEXT("RemoteControlEditor"), NSLOCTEXT("Contexts", "RemoteControlEditor", "RemoteControlEditor Plugin"), NAME_None, FRemoteControlEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
