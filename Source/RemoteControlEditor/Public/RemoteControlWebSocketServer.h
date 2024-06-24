#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "INetworkingWebSocket.h"
#include "RemoteControlWebSocketServer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRemoteAction, Log, All)

USTRUCT()
struct FActionCommand
{
	GENERATED_BODY()

	UPROPERTY()
	FString Action;
};

USTRUCT()
struct FEditorState
{
	GENERATED_BODY()

	UPROPERTY()
	FString PIESessionState;

	UPROPERTY()
	FString PIESessionType;

	UPROPERTY()
	int64 Time = 0;
};

USTRUCT()
struct FMyStruct
{
	GENERATED_BODY()
};

UCLASS(config = RemoteControlEditor)
class REMOTECONTROLEDITOR_API URemoteControlWebSocketServer : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnJsonRecieved, const FString&, Payload);

	UPROPERTY(BlueprintAssignable)
	FOnJsonRecieved OnJsonReceived;

protected:
	void OnWebSocketClientConnected(INetworkingWebSocket* ClientWebSocket); // to the server.

	virtual void ReceivedRawPacket(void* Data, int32 Count, INetworkingWebSocket* ClientWebSocket);

	UPROPERTY(config)
	bool bUseSubsystem = true;

	UPROPERTY(config)
	int32 WebSocketPort;
	
	UPROPERTY(config)
	int32 CheckSessionInterval;
	
	virtual void HandleMessage(INetworkingWebSocket* ClientWebSocket, const FString& Payload);

	virtual const TSharedRef<FUICommandInfo> GetLastPlaySessionCommand();

	virtual FString GetLastPlaySessionType();

	template <typename T>
	void SendEditorState(INetworkingWebSocket* ClientWebSocket, const T& EditorState);

	FString GetSessionState();

	void SendSessionState(INetworkingWebSocket* ClientWebSocket);
	
	void CheckAndSendSessionState();

private:
	TUniquePtr<class IWebSocketServer> ServerWebSocket;
	TArray<INetworkingWebSocket*> WebSocketClients;

	FString LastSessionState;

	/** Delegate for callbacks to GameThreadTick */
	FTSTicker::FDelegateHandle TickHandle;

	int64 TickTime;
};
