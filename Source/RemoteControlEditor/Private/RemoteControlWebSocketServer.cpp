// Fill out your copyright notice in the Description page of Project Settings.


#include "RemoteControlWebSocketServer.h"

#include "INetworkingWebSocket.h"
#include "IWebSocketNetworkingModule.h"
#include "IWebSocketServer.h"
#include "JsonObjectConverter.h"
#include "LevelEditor.h"
#include "WebSocketNetworkingDelegates.h"
#include "Kismet2/DebuggerCommands.h"

DEFINE_LOG_CATEGORY(LogRemoteAction)

bool URemoteControlWebSocketServer::ShouldCreateSubsystem(UObject* Outer) const
{
	return bUseSubsystem;
}

void URemoteControlWebSocketServer::Initialize(FSubsystemCollectionBase& Collection)
{
	ServerWebSocket = FModuleManager::Get().LoadModuleChecked<IWebSocketNetworkingModule>(TEXT("WebSocketNetworking")).
	                                        CreateServer();

	FWebSocketClientConnectedCallBack CallBack;
	CallBack.BindUObject(this, &URemoteControlWebSocketServer::OnWebSocketClientConnected);

	if (!ServerWebSocket->Init(WebSocketPort, CallBack))
	{
		UE_LOG(LogClass, Warning, TEXT("ServerWebSocket Init FAIL"));
		ServerWebSocket.Reset();
		CallBack.Unbind();
		return;
	}

	TickHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([&](float Time)
	{
		if (ServerWebSocket)
		{
			ServerWebSocket->Tick();
			int64 Now = FDateTime::Now().GetTicks();
			if (Now - TickTime >= CheckSessionInterval)
			{
				CheckAndSendSessionState();
			}
			TickTime = Now;
			return true;
		}
		else
		{
			return false;
		}
	}));
}

void URemoteControlWebSocketServer::Deinitialize()
{
	ServerWebSocket = nullptr;

	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
}

void URemoteControlWebSocketServer::OnWebSocketClientConnected(INetworkingWebSocket* ClientWebSocket)
{
	FWebSocketPacketReceivedCallBack CallBack;
	CallBack.BindLambda([this, ClientWebSocket](void* Data, int32 Count)
	{
		ReceivedRawPacket(Data, Count, ClientWebSocket);
	});
	ClientWebSocket->SetReceiveCallBack(CallBack);
	FWebSocketInfoCallBack ClosedCallBack;
	ClosedCallBack.BindLambda([this, ClientWebSocket]()
	{
		WebSocketClients.Remove(ClientWebSocket);
	});
	ClientWebSocket->SetSocketClosedCallBack(ClosedCallBack);
	WebSocketClients.Add(ClientWebSocket);
	SendSessionState(ClientWebSocket);
}

void URemoteControlWebSocketServer::ReceivedRawPacket(void* Data, int32 Count,
                                                      INetworkingWebSocket* ClientWebSocket)
{
	if (Count == 0) // nothing to process
	{
		return;
	}

	TArray<uint8> MessageData;
	MessageData.SetNumUninitialized(Count + 1);
	FMemory::Memcpy(MessageData.GetData(), Data, Count);
	MessageData[Count] = '\0';

	const FString JSonData = UTF8_TO_TCHAR(MessageData.GetData());
	HandleMessage(ClientWebSocket, JSonData);
	OnJsonReceived.Broadcast(JSonData);
}

void URemoteControlWebSocketServer::HandleMessage(INetworkingWebSocket* ClientWebSocket, const FString& Payload)
{
	TSharedPtr<FJsonObject> JsonObject;
	auto Reader = TJsonReaderFactory<>::Create(Payload);
	if (!FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		UE_LOG(LogRemoteAction, Warning, TEXT("Can't deserialize, json string: %s, payload length: %d"), *Payload,
		       Payload.Len());
		return;
	}
	FActionCommand ActionCommand;
	if (!FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), FActionCommand::StaticStruct(),
	                                               &ActionCommand))
	{
		UE_LOG(LogRemoteAction, Warning, TEXT("Can't convert json object to FActionCommand, json string: %s"),
		       *Payload);
	}
	if (ActionCommand.Action == TEXT("session_state"))
	{
		SendSessionState(ClientWebSocket);
	}
	if (ActionCommand.Action == TEXT("play"))
	{
		TSharedRef<FUICommandInfo> Command = GetLastPlaySessionCommand();
		if (FPlayWorldCommands::GlobalPlayWorldActions->TryExecuteAction(GetLastPlaySessionCommand()))
		{
			SendSessionStateAfterChanged(TEXT("running"));
		}
		else
		{
			SendSessionStateAfterChanged(TEXT("stoped"));
		}
	}
	if (ActionCommand.Action == TEXT("stop"))
	{
		TSharedRef<FUICommandInfo> Command = FPlayWorldCommands::Get().StopPlaySession.ToSharedRef();
		if (FPlayWorldCommands::GlobalPlayWorldActions->TryExecuteAction(Command))
		{
			SendSessionStateAfterChanged(TEXT("stoped"));
		}
		else
		{
			SendSessionStateAfterChanged(TEXT("running"));
		}
	}
}

const TSharedRef<FUICommandInfo> URemoteControlWebSocketServer::GetLastPlaySessionCommand()
{
	const ULevelEditorPlaySettings* PlaySettings = GetDefault<ULevelEditorPlaySettings>();

	const FPlayWorldCommands& Commands = FPlayWorldCommands::Get();
	TSharedRef<FUICommandInfo> Command = Commands.PlayInViewport.ToSharedRef();

	switch (PlaySettings->LastExecutedPlayModeType)
	{
	case PlayMode_InViewPort:
		Command = Commands.PlayInViewport.ToSharedRef();
		break;

	case PlayMode_InEditorFloating:
		Command = Commands.PlayInEditorFloating.ToSharedRef();
		break;

	case PlayMode_InMobilePreview:
		Command = Commands.PlayInMobilePreview.ToSharedRef();
		break;

	case PlayMode_InTargetedMobilePreview:
		{
			// Scan through targeted mobile preview commands to find our match.
			for (auto PreviewerCommand : Commands.PlayInTargetedMobilePreviewDevices)
			{
				FName LastExecutedPIEPreviewDevice = FName(*PlaySettings->LastExecutedPIEPreviewDevice);
				if (PreviewerCommand->GetCommandName() == LastExecutedPIEPreviewDevice)
				{
					Command = PreviewerCommand.ToSharedRef();
					break;
				}
			}
			break;
		}

	case PlayMode_InVulkanPreview:
		Command = Commands.PlayInVulkanPreview.ToSharedRef();
		break;

	case PlayMode_InNewProcess:
		Command = Commands.PlayInNewProcess.ToSharedRef();
		break;

	case PlayMode_InVR:
		Command = Commands.PlayInVR.ToSharedRef();
		break;

	case PlayMode_Simulate:
		Command = Commands.Simulate.ToSharedRef();
	}

	return Command;
}

FString URemoteControlWebSocketServer::GetLastPlaySessionType()
{
	const ULevelEditorPlaySettings* PlaySettings = GetDefault<ULevelEditorPlaySettings>();
	FString Type;
	switch (PlaySettings->LastExecutedPlayModeType)
	{
	case PlayMode_InViewPort:
		Type = TEXT("ViewPort");
		break;
	case PlayMode_InEditorFloating:
		Type = TEXT("EditorFloating");
		break;
	case PlayMode_InMobilePreview:
		Type = TEXT("MobilePreview");
		break;
	case PlayMode_InTargetedMobilePreview:
		{
			Type = TEXT("MobilePreview");
			const FPlayWorldCommands& Commands = FPlayWorldCommands::Get();
			// Scan through targeted mobile preview commands to find our match.
			// for (auto PreviewerCommand : Commands.PlayInTargetedMobilePreviewDevices)
			// {
			// 	FName LastExecutedPIEPreviewDevice = FName(*PlaySettings->LastExecutedPIEPreviewDevice);
			// 	if (PreviewerCommand->GetCommandName() == LastExecutedPIEPreviewDevice)
			// 	{
			// 		Command = PreviewerCommand.ToSharedRef();
			// 		break;
			// 	}
			// }
			break;
		}

	case PlayMode_InVulkanPreview:
		Type = TEXT("VulkanPreview");
		break;

	case PlayMode_InNewProcess:
		Type = TEXT("NewProcess");
		break;

	case PlayMode_InVR:
		Type = TEXT("VR");
		break;

	case PlayMode_Simulate:
		Type = TEXT("Simulate");
	}
	return Type;
}

FString URemoteControlWebSocketServer::GetSessionState()
{
	return GEditor->PlayWorld == nullptr ? TEXT("stoped") : TEXT("running");
}

void URemoteControlWebSocketServer::SendSessionState(INetworkingWebSocket* ClientWebSocket)
{
	FEditorState EditorState;
	EditorState.Time = FDateTime::Now().GetTicks() / ETimespan::TicksPerMillisecond;
	EditorState.PIESessionType = GetLastPlaySessionType();
	EditorState.PIESessionState = GetSessionState();
	SendEditorState(ClientWebSocket, EditorState);
}

void URemoteControlWebSocketServer::CheckAndSendSessionState()
{
	FString NowSessionState = GetSessionState();
	if (LastSessionState == NowSessionState || WebSocketClients.IsEmpty())
	{
		return;
	}
	LastSessionState = NowSessionState;
	FEditorState EditorState;
	EditorState.Time = FDateTime::Now().GetTicks() / ETimespan::TicksPerMillisecond;
	EditorState.PIESessionType = GetLastPlaySessionType();
	EditorState.PIESessionState = NowSessionState;
	for (auto Client : WebSocketClients)
	{
		SendEditorState(Client, EditorState);
	}
}

void URemoteControlWebSocketServer::SendSessionStateAfterChanged(const FString& NewSessionState)
{
	TickTime = FDateTime::Now().GetTicks();
	FEditorState EditorState;
	EditorState.Time = FDateTime::Now().GetTicks() / ETimespan::TicksPerMillisecond;
	EditorState.PIESessionType = GetLastPlaySessionType();
	EditorState.PIESessionState = NewSessionState;
	for (auto Client : WebSocketClients)
	{
		SendEditorState(Client, EditorState);
	}
}

template <typename T>
void URemoteControlWebSocketServer::SendEditorState(INetworkingWebSocket* ClientWebSocket,
                                                    const T& EditorState)
{
	if (!ClientWebSocket)
	{
		return;
	}
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(T::StaticStruct(), &EditorState, JsonString, 0, 0, 0,
	                                                nullptr, false);
	uint8* UTF8String = (uint8*)TCHAR_TO_UTF8(*JsonString);
	if (UTF8String == nullptr)
	{
		return;
	}
	int32 UTF8StringLength = 0;
	while (UTF8String[UTF8StringLength] != '\0')
	{
		++UTF8StringLength;
	}
	ClientWebSocket->Send(UTF8String, UTF8StringLength, false);
}
