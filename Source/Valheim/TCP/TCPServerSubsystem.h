// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TCPServerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTCPClientConnected, int32, ClientId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTCPClientDisconnected, int32, ClientId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnServerChatReceived, int32, ClientId, const FString&, UserId, const FString&, Message);

class FSocket;
class FTCPRecvWorker;
class FTCPAcceptWorker;
class FRunnableThread;

struct FTCPClientInfo
{
	FSocket* Socket;
	FTCPRecvWorker* RecvWorker;
	FRunnableThread* RecvThread;
	TQueue<TArray<uint8>> RecvQueue;
	int32 ClientId;
};

UCLASS()
class VALHEIM_API UTCPServerSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:

	virtual void Deinitialize() override;

	// 호스트(리슨서버)에서 채팅용 TCP 서버를 오픈할 때 호출
	UFUNCTION(BlueprintCallable, Category = "TCP")
	bool StartListen(int32 Port);

	UFUNCTION(BlueprintCallable, Category = "TCP")
	void StopListen();

	UFUNCTION(BlueprintCallable, Category = "TCP")
	bool IsListening() const;

	// 클라이언트로부터 받은 채팅을 모든 클라이언트에게 다시 뿌릴 때 사용
	UFUNCTION(BlueprintCallable, Category = "TCP")
	void BroadcastChat(const FString& UserId, const FString& Message);

	// 호스트 자기 자신이 채팅을 보낼 때 : 소켓을 거치지 않고 바로 브로드캐스트 + 자기 UI 갱신
	UFUNCTION(BlueprintCallable, Category = "TCP")
	void SendLocalChat(const FString& UserId, const FString& Message);

	UPROPERTY(BlueprintAssignable, Category = "TCP")
	FOnTCPClientConnected OnClientConnected;

	UPROPERTY(BlueprintAssignable, Category = "TCP")
	FOnTCPClientDisconnected OnClientDisconnected;

	UPROPERTY(BlueprintAssignable, Category = "TCP")
	FOnServerChatReceived OnChatReceived;

	// FTCPAcceptWorker(Accept 스레드)에서 호출됨. 게임 스레드가 아니므로 무거운 작업 금지.
	void RegisterClient(FSocket* NewClientSocket);

private:

	FSocket* ListenSocket = nullptr;

	FTCPAcceptWorker* AcceptWorker = nullptr;
	FRunnableThread* AcceptThread = nullptr;

	// ClientId -> 클라이언트 정보. Accept 스레드(쓰기)와 게임 스레드(읽기)가 동시에 접근하므로 Lock 필요.
	TMap<int32, FTCPClientInfo*> Clients;
	FCriticalSection ClientsLock;

	// Accept 스레드에서 새로 접속한 ClientId를 여기 쌓아두면, Tick에서 게임 스레드로 안전하게 델리게이트 브로드캐스트
	TQueue<int32> PendingConnectedIds;

	int32 NextClientId = 1;

	bool SendToClient(int32 ClientId, const uint8* Body, uint32 BodyLength);

	void DisconnectClient(int32 ClientId);

	void DispatchPacket(int32 ClientId, const TArray<uint8>& Buffer);

	// Inherited via FTickableGameObject
	TStatId GetStatId() const override;

	virtual void Tick(float DeltaTime) override;
};
