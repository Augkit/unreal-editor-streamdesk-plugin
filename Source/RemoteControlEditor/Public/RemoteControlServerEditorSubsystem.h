
#pragma once

#include "CoreMinimal.h"
#include "EditorSubsystem.h"
#include "INetworkingWebSocket.h"
#include "RemoteControlServerEditorSubsystem.generated.h"

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
};

UCLASS(config = RemoteControlServer)
class REMOTECONTROLEDITOR_API URemoteControlServerEditorSubsystem : public UEditorSubsystem
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
	bool bUseSubsystem;

	UPROPERTY(config)
	uint32 WebSocketPort;

	virtual void HandleMessage(INetworkingWebSocket* ClientWebSocket, const FString& Payload);

	virtual void SendEditorState(INetworkingWebSocket* ClientWebSocket, const FEditorState& EditorState);

private:

	TUniquePtr<class IWebSocketServer> ServerWebSocket;

	/** Delegate for callbacks to GameThreadTick */
	FTSTicker::FDelegateHandle TickHandle;
	
};
