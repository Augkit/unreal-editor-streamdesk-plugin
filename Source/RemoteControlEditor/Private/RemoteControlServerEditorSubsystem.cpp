// Fill out your copyright notice in the Description page of Project Settings.


#include "RemoteControlServerEditorSubsystem.h"

#include "INetworkingWebSocket.h"
#include "IWebSocketNetworkingModule.h"
#include "IWebSocketServer.h"
#include "JsonObjectConverter.h"
#include "WebSocketNetworkingDelegates.h"

DEFINE_LOG_CATEGORY(LogRemoteAction)

bool URemoteControlServerEditorSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return bUseSubsystem;
}

void URemoteControlServerEditorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	ServerWebSocket = FModuleManager::Get().LoadModuleChecked<IWebSocketNetworkingModule>(TEXT("WebSocketNetworking")).
	                                        CreateServer();

	FWebSocketClientConnectedCallBack CallBack;
	CallBack.BindUObject(this, &URemoteControlServerEditorSubsystem::OnWebSocketClientConnected);

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
			return true;
		}
		else
		{
			return false;
		}
	}));
}

void URemoteControlServerEditorSubsystem::Deinitialize()
{
	ServerWebSocket = nullptr;

	if (TickHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(TickHandle);
		TickHandle.Reset();
	}
}

void URemoteControlServerEditorSubsystem::OnWebSocketClientConnected(INetworkingWebSocket* ClientWebSocket)
{
	FWebSocketPacketReceivedCallBack CallBack;
	CallBack.BindLambda([this, ClientWebSocket](void* Data, int32 Count)
	{
		ReceivedRawPacket(Data, Count, ClientWebSocket);
	});
	ClientWebSocket->SetReceiveCallBack(CallBack);
}

void URemoteControlServerEditorSubsystem::ReceivedRawPacket(void* Data, int32 Count,
                                                            INetworkingWebSocket* ClientWebSocket)
{
	if (Count == 0) // nothing to process
	{
		return;
	}

	const uint8* DataRef = reinterpret_cast<uint8*>(Data);

	const TArray<uint8> MessageData(DataRef, Count);

	const FString JSonData = UTF8_TO_TCHAR(MessageData.GetData());

	HandleMessage(ClientWebSocket, JSonData);
	OnJsonReceived.Broadcast(JSonData);
}

void URemoteControlServerEditorSubsystem::HandleMessage(INetworkingWebSocket* ClientWebSocket, const FString& Payload)
{
	TSharedPtr<FJsonObject> JsonObject;
	auto Reader = TJsonReaderFactory<>::Create(Payload);
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		TArray<FActionCommand> Array;
		int Index = 0;
		while (true)
		{
			const TSharedPtr<FJsonObject>* JsonItem;
			if (JsonObject->TryGetObjectField(FString::Printf(TEXT("%i"), Index), JsonItem))
			{
				Array.Emplace();
				FJsonObjectConverter::JsonObjectToUStruct<FActionCommand>(JsonItem->ToSharedRef(), &Array[Index]);
				Index++;
			}
			else
				break;
		}
		if (Array.Num() < 0)
		{
			UE_LOG(LogRemoteAction, Warning, TEXT("Deserialized json is empty, json string: %s"), *Payload);
			return;
		}
		for (const FActionCommand& ActionCommand : Array)
		{
			if (ActionCommand.Action == TEXT("play"))
			{
				FEditorState EditorState;
				EditorState.PIESessionState = TEXT("running");
				SendEditorState(ClientWebSocket, EditorState);
			}
		}
	}
	else
	{
		UE_LOG(LogRemoteAction, Warning, TEXT("Can't deserialize, json string: %s"), *Payload);
	}
}

void URemoteControlServerEditorSubsystem::SendEditorState(INetworkingWebSocket* ClientWebSocket,
                                                          const FEditorState& EditorState)
{
	if (!ClientWebSocket)
	{
		return;
	}
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(FEditorState::StaticStruct(), &EditorState, JsonString);
	int32 Utf8Length = FTCHARToUTF8_Convert::ConvertedLength(*JsonString, JsonString.Len());
	TArray<uint8> Buffer;
	Buffer.SetNumUninitialized(Utf8Length);
	FTCHARToUTF8_Convert::Convert(reinterpret_cast<UTF8CHAR*>(Buffer.GetData()), Buffer.Num(), *JsonString, JsonString.Len());
	ClientWebSocket->Send(Buffer.GetData(), Buffer.Num());
}
