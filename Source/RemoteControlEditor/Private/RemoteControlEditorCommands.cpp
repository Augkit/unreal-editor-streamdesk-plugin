// Copyright Epic Games, Inc. All Rights Reserved.

#include "RemoteControlEditorCommands.h"

#define LOCTEXT_NAMESPACE "FRemoteControlEditorModule"

void FRemoteControlEditorCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "RemoteControlEditor", "Execute RemoteControlEditor action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
