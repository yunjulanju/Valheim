// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AINpcSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAIChatReply, const FString&, PlayerId, const FString&, NpcId, const FString&, Reply);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnAIChatError, const FString&, PlayerId, const FString&, NpcId, const FString&, ErrorCode);

class IWebSocket;

UCLASS()
class VALHEIM_API UAINpcSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	bool Connect(const FString& Host, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	bool IsAIServerConnected() const;

	UFUNCTION(BlueprintCallable, Category = "AINpc")
	void RequestChat(const FString& PlayerId, const FString& NpcId, const FString& Message);

	UPROPERTY(BlueprintAssignable, Category = "AINpc")
	FOnAIChatReply OnAIChatReply;

	UPROPERTY(BlueprintAssignable, Category = "AINpc")
	FOnAIChatError OnAIChatError;

	static FString BuildChatRequestJson(const FString& PlayerId, const FString& NpcId, const FString& Message);
	static bool ParseServerMessage(const FString& JsonText, FString& OutType, FString& OutPlayerId, FString& OutNpcId, FString& OutPayload);

private:
	TSharedPtr<IWebSocket> Socket;

	void HandleMessage(const FString& MessageString);
};
