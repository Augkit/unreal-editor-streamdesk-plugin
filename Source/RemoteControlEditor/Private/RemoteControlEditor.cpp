// Copyright Epic Games, Inc. All Rights Reserved.

#include "RemoteControlEditor.h"

#include "ISettingsModule.h"
#include "RemoteControlEditorStyle.h"
#include "RemoteControlEditorCommands.h"
#include "RemoteControlEditorSettings.h"
#include "RemoteControlWebSocketServer.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FRemoteControlEditorModule"

void FRemoteControlEditorModule::StartupModule()
{
	FRemoteControlEditorStyle::Initialize();
	FRemoteControlEditorStyle::ReloadTextures();

	FRemoteControlEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FRemoteControlEditorCommands::Get().RestartAction,
		FExecuteAction::CreateRaw(this, &FRemoteControlEditorModule::StartWebSocketServer),
		FCanExecuteAction()
	);
	PluginCommands->MapAction(
		FRemoteControlEditorCommands::Get().SettingsAction,
		FExecuteAction::CreateRaw(this, &FRemoteControlEditorModule::OpenSettingsPanel),
		FCanExecuteAction()
	);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRemoteControlEditorModule::RegisterMenus));

	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Editor", "Plugins", "RemoteControlEditorSettings",
		                                 LOCTEXT("EditorSettingsName", "Remote Control Editor"),
		                                 LOCTEXT("EditorSettingsDesc", ""),
		                                 GetMutableDefault<URemoteControlEditorSettings>());
	}
}

void FRemoteControlEditorModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FRemoteControlEditorStyle::Shutdown();

	FRemoteControlEditorCommands::Unregister();
}

void FRemoteControlEditorModule::InitWebSocketServer()
{
	URemoteControlWebSocketServer* WebSocketServerSubsystem = GEditor->GetEditorSubsystem<
		URemoteControlWebSocketServer>();
	if (WebSocketServerSubsystem != nullptr)
	{
		WebSocketServerSubsystem->OnWebSocketServerOpen.
		                          AddRaw(this, &FRemoteControlEditorModule::HandleWebSocketServerOpen);
		WebSocketServerSubsystem->OnWebSocketServerClosed.AddRaw(
			this, &FRemoteControlEditorModule::HandleWebSocketServerClosed);
	}
	StartWebSocketServer();
}

void FRemoteControlEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("RCEComboButton");
			{
				FToolMenuEntry Entry = FToolMenuEntry::InitComboButton(
					"RCEComboButton",
					FUIAction(),
					FOnGetContent::CreateLambda([&]()-> TSharedRef<SWidget, ESPMode::ThreadSafe>
					{
						FMenuBuilder MenuBuilder(true, PluginCommands);
						MenuBuilder.BeginSection("RemoteControlEditor",LOCTEXT("WebSocket", "WebSocket"));
						MenuBuilder.AddMenuEntry(FRemoteControlEditorCommands::Get().RestartAction);
						MenuBuilder.EndSection();
						MenuBuilder.AddMenuEntry(FRemoteControlEditorCommands::Get().SettingsAction);
						return MenuBuilder.MakeWidget();
					}),
					TAttribute<FText>(),
					LOCTEXT("RemoteControlEditorMenuTooltip", "WebSocket Status"),
					FSlateIcon(FRemoteControlEditorStyle::GetStyleSetName(), "RemoteControlEditor.PluginAction"),
					false,
					"RemoteControlEditorMenu"
				);
				Entry.StyleNameOverride = "CalloutToolbar";
				Entry.SetCommandList(PluginCommands);
				Section.AddEntry(Entry);
			}
		}
	}
}

void FRemoteControlEditorModule::StartWebSocketServer()
{
	URemoteControlWebSocketServer* WebSocketServerSubsystem = GEditor->GetEditorSubsystem<
		URemoteControlWebSocketServer>();
	if (WebSocketServerSubsystem == nullptr)
	{
		return;
	}
	WebSocketServerSubsystem->RestartWebSocketServer();
}

void FRemoteControlEditorModule::HandleWebSocketServerOpen()
{
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("RCEComboButton");
	FToolMenuEntry* Entry = Section.FindEntry("RCEComboButton");
	Entry->Icon = FSlateIcon(FRemoteControlEditorStyle::GetStyleSetName(),
	                         "RemoteControlEditor.PluginAction.Connected");
}

void FRemoteControlEditorModule::HandleWebSocketServerClosed()
{
	UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.User");
	FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("RCEComboButton");
	FToolMenuEntry* Entry = Section.FindEntry("RCEComboButton");
	Entry->Icon = FSlateIcon(FRemoteControlEditorStyle::GetStyleSetName(), "RemoteControlEditor.PluginAction");
}

void FRemoteControlEditorModule::OpenSettingsPanel()
{
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->ShowViewer("Editor", "Plugins", "RemoteControlEditorSettings");
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRemoteControlEditorModule, RemoteControlEditor)
