// Copyright Epic Games, Inc. All Rights Reserved.

#include "RemoteControlEditorCommands.h"

#define LOCTEXT_NAMESPACE "FRemoteControlEditorModule"

void FRemoteControlEditorCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "RemoteControlEditor", "Open Tool Menu", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RestartAction, "Restart WebSocket Server", "Restart WebSocket Server", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(SettingsAction, "Open Settings Panel", "Open Settings Panel", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
