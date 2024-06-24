// Copyright Epic Games, Inc. All Rights Reserved.

#include "RemoteControlEditor.h"
#include "RemoteControlEditorStyle.h"
#include "RemoteControlEditorCommands.h"
#include "Misc/MessageDialog.h"
#include "ToolMenus.h"

static const FName RemoteControlEditorTabName("RemoteControlEditor");

#define LOCTEXT_NAMESPACE "FRemoteControlEditorModule"

void FRemoteControlEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FRemoteControlEditorStyle::Initialize();
	FRemoteControlEditorStyle::ReloadTextures();

	FRemoteControlEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FRemoteControlEditorCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FRemoteControlEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRemoteControlEditorModule::RegisterMenus));
}

void FRemoteControlEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FRemoteControlEditorStyle::Shutdown();

	FRemoteControlEditorCommands::Unregister();
}

void FRemoteControlEditorModule::PluginButtonClicked()
{
	// Put your "OnButtonClicked" stuff here
	FText DialogText = FText::Format(
							LOCTEXT("PluginButtonDialogText", "Add code to {0} in {1} to override this button's actions"),
							FText::FromString(TEXT("FRemoteControlEditorModule::PluginButtonClicked()")),
							FText::FromString(TEXT("RemoteControlEditor.cpp"))
					   );
	FMessageDialog::Open(EAppMsgType::Ok, DialogText);
}

void FRemoteControlEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FRemoteControlEditorCommands::Get().PluginAction, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FRemoteControlEditorCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRemoteControlEditorModule, RemoteControlEditor)