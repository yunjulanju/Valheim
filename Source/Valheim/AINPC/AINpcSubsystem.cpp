// Fill out your copyright notice in the Description page of Project Settings.

#include "AINPC/AINpcSubsystem.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FString UAINpcSubsystem::BuildChatRequestJson(const FString& PlayerId, const FString& NpcId, const FString& Message)
{
	TSharedRef<FJsonObject> Json = MakeShared<FJsonObject>();
	Json->SetStringField(TEXT("type"), TEXT("chat"));
	Json->SetStringField(TEXT("player_id"), PlayerId);
	Json->SetStringField(TEXT("npc_id"), NpcId);
	Json->SetStringField(TEXT("message"), Message);

	FString Output;
	TSharedRef<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer = TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&Output);
	FJsonSerializer::Serialize(Json, Writer);
	return Output;
}

bool UAINpcSubsystem::ParseServerMessage(const FString& JsonText, FString& OutType, FString& OutPlayerId, FString& OutNpcId, FString& OutPayload)
{
	TSharedPtr<FJsonObject> Json;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonText);
	if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
	{
		return false;
	}

	if (!Json->TryGetStringField(TEXT("type"), OutType))
	{
		return false;
	}

	Json->TryGetStringField(TEXT("player_id"), OutPlayerId);
	Json->TryGetStringField(TEXT("npc_id"), OutNpcId);

	if (OutType == TEXT("chat_reply"))
	{
		return Json->TryGetStringField(TEXT("reply"), OutPayload);
	}
	if (OutType == TEXT("error"))
	{
		return Json->TryGetStringField(TEXT("error"), OutPayload);
	}
	return false;
}

void UAINpcSubsystem::Deinitialize()
{
	if (Socket.IsValid() && Socket->IsConnected())
	{
		Socket->Close();
	}
	Socket.Reset();
	Super::Deinitialize();
}

bool UAINpcSubsystem::Connect(const FString& Host, int32 Port)
{
	if (Socket.IsValid() && Socket->IsConnected())
	{
		return true;
	}

	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
	}

	const FString Url = FString::Printf(TEXT("ws://%s:%d/ws"), *Host, Port);
	Socket = FWebSocketsModule::Get().CreateWebSocket(Url);

	Socket->OnMessage().AddUObject(this, &UAINpcSubsystem::HandleMessage);
	Socket->OnConnectionError().AddLambda([](const FString& Error)
	{
		UE_LOG(LogTemp, Warning, TEXT("UAINpcSubsystem: connection error: %s"), *Error);
	});

	Socket->Connect();
	return true;
}

bool UAINpcSubsystem::IsAIServerConnected() const
{
	return Socket.IsValid() && Socket->IsConnected();
}

void UAINpcSubsystem::RequestChat(const FString& PlayerId, const FString& NpcId, const FString& Message)
{
	if (!IsAIServerConnected())
	{
		OnAIChatError.Broadcast(PlayerId, NpcId, TEXT("not_connected"));
		return;
	}

	Socket->Send(BuildChatRequestJson(PlayerId, NpcId, Message));
}

void UAINpcSubsystem::HandleMessage(const FString& MessageString)
{
	FString Type, PlayerId, NpcId, Payload;
	if (!ParseServerMessage(MessageString, Type, PlayerId, NpcId, Payload))
	{
		UE_LOG(LogTemp, Warning, TEXT("UAINpcSubsystem: failed to parse message: %s"), *MessageString);
		return;
	}

	if (Type == TEXT("chat_reply"))
	{
		OnAIChatReply.Broadcast(PlayerId, NpcId, Payload);
	}
	else if (Type == TEXT("error"))
	{
		OnAIChatError.Broadcast(PlayerId, NpcId, Payload);
	}
}
