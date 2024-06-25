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

protected:
	UPROPERTY(config)
	bool bUseSubsystem = true;

	UPROPERTY(config)
	int32 WebSocketPort;

	UPROPERTY(config)
	int32 CheckSessionInterval;

	FRunnableThread* WebSocketWorkerThread;
};

class REMOTECONTROLEDITOR_API FWebSocketServerWorker : public FRunnable
{
	FThreadSafeCounter ExitRequest;

	uint32 WebSocketPort;
	int32 CheckSessionInterval;

	TUniquePtr<class IWebSocketServer> ServerWebSocket;
	TArray<INetworkingWebSocket*> WebSocketClients;
	FString LastSessionState;
	int64 LastTickTime = 0;

	FCriticalSection HandleMessageMutex;

public:
	FWebSocketServerWorker(uint32 InWebSocketPort, int32 CheckSessionInterval);
	virtual ~FWebSocketServerWorker() override;

	virtual void Exit() override;
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;

protected:
	void OnWebSocketClientConnected(INetworkingWebSocket* ClientWebSocket); // to the server.

	void ReceivedRawPacket(void* Data, int32 Count, INetworkingWebSocket* InClientWebSocket);

	void HandleMessage(INetworkingWebSocket* ClientWebSocket, const FString& Payload);

	virtual const TSharedRef<FUICommandInfo> GetLastPlaySessionCommand();

	virtual FString GetLastPlaySessionType();

	template <typename T>
	void SendEditorState(INetworkingWebSocket* ClientWebSocket, const T& EditorState);

	FString GetSessionState();

	void SendSessionState(INetworkingWebSocket* ClientWebSocket);

	void CheckAndSendSessionState();

	void SendSessionStateAfterChanged(const FString& NewSessionState);
};
