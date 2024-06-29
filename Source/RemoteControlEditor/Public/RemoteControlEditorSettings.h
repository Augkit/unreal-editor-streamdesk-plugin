// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "RemoteControlEditorSettings.generated.h"

UCLASS(config = RemoteControlEditor)
class REMOTECONTROLEDITOR_API URemoteControlEditorSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(config, EditAnywhere, Category= "WebSocket Server")
	int32 WebSocketPort;

	UPROPERTY(config, EditAnywhere, Category= "WebSocket Server")
	int32 CheckSessionInterval;
};
