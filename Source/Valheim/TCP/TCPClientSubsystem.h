// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TCPClientSubsystem.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTCPConnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTCPDisconnected);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChatReceived, const FString&, UserId, const FString&, Message);


class FSocket;
class FTCPRecvWorker;
class FRunnableThread;

UCLASS()
class VALHEIM_API UTCPClientSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
public:

	virtual void Deinitialize() override;


	UFUNCTION(BlueprintCallable, Category = "TCP")
	bool Connect(const FString& Host, int32 Port);

	UFUNCTION(BlueprintCallable, Category = "TCP")
	void Disconnect();

	UFUNCTION(BlueprintCallable, Category = "TCP")
	bool IsConncted() const;

	UPROPERTY(BlueprintAssignable, Category = "TCP")
	FOnTCPConnected OnTCPConnected;

	UPROPERTY(BlueprintAssignable, Category = "TCP")
	FOnTCPDisconnected OnTCPDisconnected;

	UPROPERTY(BlueprintAssignable, Category = "TCP")
	FOnChatReceived OnChatReceived;

	UFUNCTION(BlueprintCallable, Category = "TCP")
	bool SendChat(const FString& UserId, const FString& Message);

	/*UPROPERTY(BlueprintAssignable, Category = "TCP")
	FOnLoginCallback OnLogin;

	UPROPERTY(BlueprintAssignable, Category = "TCP")
	FOnSignUpCallback OnSignUp;*/


	//UFUNCTION(BlueprintCallable, Category = "TCP")
	//void SendLogin(const FString& UserID, const FString& Password);


	//UFUNCTION(BlueprintCallable, Category = "TCP")
	//void SendSignUp(const FString& UserID, const FString& Password, const FString& Name);


private:

	FTCPRecvWorker* RecvWorker = nullptr;
	FRunnableThread* RecvThread = nullptr;

	//stl::queue<std::vector<char>>
	TQueue<TArray<uint8>> RecvQueue;


	//char Buffer[1024];
	TArray<uint8> RecvBuffer;

	void RecvAll();

	bool SendAll(const uint8* Body, uint32 BodyLength);

	FSocket* ServerSocket = nullptr;

	void DispatchPacket();

	// Inherited via FTickableGameObject
	TStatId GetStatId() const override;

	virtual void Tick(float DeltaTime) override;
	
};
