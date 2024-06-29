#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "INetworkingWebSocket.h"
#include "RemoteControlWebSocketServer.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogRemoteAction, Log, All)

DECLARE_MULTICAST_DELEGATE(FOnWebSocketServerOpen)
DECLARE_MULTICAST_DELEGATE(FOnWebSocketServerClosed)

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

UCLASS()
class REMOTECONTROLEDITOR_API URemoteControlWebSocketServer : public UEditorSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	bool StartWebSocketServer();
	bool RestartWebSocketServer();
	void StopWebSocketServer();

	FOnWebSocketServerOpen OnWebSocketServerOpen;
	FOnWebSocketServerClosed OnWebSocketServerClosed;

protected:
	TUniquePtr<class FWebSocketServerThread> WebSocketServerThread;
};

class REMOTECONTROLEDITOR_API FWebSocketServerThread : public FRunnable
{
protected:
	FThreadSafeCounter ExitRequest;

	uint32 WebSocketPort;
	int32 CheckSessionInterval;

	FRunnableThread* Thread = nullptr;

	TUniquePtr<class IWebSocketServer> ServerWebSocket = nullptr;
	TArray<INetworkingWebSocket*> WebSocketClients;
	FString LastSessionState;
	int64 LastTickTime = 0;

	FCriticalSection HandleMessageMutex;

public:
	FWebSocketServerThread(uint32 InWebSocketPort, int32 CheckSessionInterval);
	virtual ~FWebSocketServerThread() override;

	bool StartThread();
	void StopThread();

	virtual uint32 Run() override;
	virtual void Stop() override;

	FOnWebSocketServerOpen OnWebSocketServerOpen;
	FOnWebSocketServerClosed OnWebSocketServerClosed;

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
