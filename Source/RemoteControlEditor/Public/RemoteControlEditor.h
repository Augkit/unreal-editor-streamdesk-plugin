// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FToolBarBuilder;
class FMenuBuilder;

class FRemoteControlEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void InitWebSocketServer();
	
private:

	void RegisterMenus();

	void StartWebSocketServer();
	void HandleWebSocketServerOpen();
	void HandleWebSocketServerClosed();

	void OpenSettingsPanel();
private:
	TSharedPtr<class FUICommandList> PluginCommands;
};
