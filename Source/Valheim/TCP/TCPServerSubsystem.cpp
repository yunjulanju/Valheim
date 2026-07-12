// Fill out your copyright notice in the Description page of Project Settings.


#include "TCP/TCPServerSubsystem.h"
#include "TCP/TCPAcceptWorker.h"
#include "TCP/TCPRecvWorker.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "IPAddress.h"
#include "HAL/RunnableThread.h"

#include "UserPacket_generated.h"

bool UTCPServerSubsystem::StartListen(int32 Port)
{
	if (ListenSocket)
	{
		UE_LOG(LogTemp, Warning, TEXT("Already listening."));
		return false;
	}

	ISocketSubsystem* SocketSubSystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	TSharedRef<FInternetAddr> Addr = SocketSubSystem->CreateInternetAddr();
	Addr->SetAnyAddress(); // == INADDR_ANY
	Addr->SetPort(Port);

	ListenSocket = SocketSubSystem->CreateSocket(NAME_Stream, TEXT("TCPServerListen"), Addr->GetProtocolType());
	if (!ListenSocket)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateSocket Failed."));
		return false;
	}

	ListenSocket->SetReuseAddr(true);

	if (!ListenSocket->Bind(*Addr))
	{
		UE_LOG(LogTemp, Warning, TEXT("Bind Failed."));
		return false;
	}

	if (!ListenSocket->Listen(8)) // backlog
	{
		UE_LOG(LogTemp, Warning, TEXT("Listen Failed."));
		return false;
	}

	// Accept 워커가 폴링 방식(HasPendingConnection)으로 도니 NonBlocking으로 둔다.
	ListenSocket->SetNonBlocking(true);

	AcceptWorker = new FTCPAcceptWorker(ListenSocket, this);
	AcceptThread = FRunnableThread::Create(AcceptWorker, TEXT("TCPAcceptWorker"));

	UE_LOG(LogTemp, Warning, TEXT("TCP Chat Server Listening on port %d"), Port);

	return true;
}

void UTCPServerSubsystem::StopListen()
{
	if (AcceptWorker)
	{
		AcceptWorker->Stop();
	}

	if (AcceptThread)
	{
		AcceptThread->Kill(true);
		delete AcceptThread;
		AcceptThread = nullptr;
	}

	if (AcceptWorker)
	{
		delete AcceptWorker;
		AcceptWorker = nullptr;
	}

	{
		FScopeLock Lock(&ClientsLock);

		ISocketSubsystem* SocketSubSystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

		for (auto& Pair : Clients)
		{
			FTCPClientInfo* Info = Pair.Value;

			if (Info->RecvWorker)
			{
				Info->RecvWorker->Stop();
			}

			if (Info->Socket)
			{
				Info->Socket->Close();
			}

			if (Info->RecvThread)
			{
				Info->RecvThread->Kill(true);
				delete Info->RecvThread;
			}

			if (Info->RecvWorker)
			{
				delete Info->RecvWorker;
			}

			if (Info->Socket)
			{
				SocketSubSystem->DestroySocket(Info->Socket);
			}

			delete Info;
		}

		Clients.Empty();
	}

	if (ListenSocket)
	{
		ISocketSubsystem* SocketSubSystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

		ListenSocket->Close();
		SocketSubSystem->DestroySocket(ListenSocket);
		ListenSocket = nullptr;
	}
}

void UTCPServerSubsystem::Deinitialize()
{
	StopListen();

	Super::Deinitialize();
}

bool UTCPServerSubsystem::IsListening() const
{
	return ListenSocket != nullptr;
}

void UTCPServerSubsystem::RegisterClient(FSocket* NewClientSocket)
{
	// 주의: 이 함수는 FTCPAcceptWorker 스레드(게임 스레드가 아님)에서 호출됩니다.
	if (!NewClientSocket)
	{
		return;
	}

	NewClientSocket->SetNonBlocking(false); // RecvWorker가 blocking Recv로 도니까

	FTCPClientInfo* Info = new FTCPClientInfo();
	Info->Socket = NewClientSocket;
	Info->RecvWorker = new FTCPRecvWorker(NewClientSocket, Info->RecvQueue);

	{
		FScopeLock Lock(&ClientsLock);
		Info->ClientId = NextClientId++;
		Info->RecvThread = FRunnableThread::Create(Info->RecvWorker, *FString::Printf(TEXT("TCPClientRecv_%d"), Info->ClientId));
		Clients.Add(Info->ClientId, Info);
	}

	PendingConnectedIds.Enqueue(Info->ClientId);

	UE_LOG(LogTemp, Warning, TEXT("Client %d connected."), Info->ClientId);
}

void UTCPServerSubsystem::DisconnectClient(int32 ClientId)
{
	FTCPClientInfo* Info = nullptr;

	{
		FScopeLock Lock(&ClientsLock);
		FTCPClientInfo** Found = Clients.Find(ClientId);
		if (!Found)
		{
			return;
		}
		Info = *Found;
		Clients.Remove(ClientId);
	}

	if (Info->RecvWorker)
	{
		Info->RecvWorker->Stop();
	}

	if (Info->Socket)
	{
		Info->Socket->Close();
	}

	if (Info->RecvThread)
	{
		Info->RecvThread->Kill(true);
		delete Info->RecvThread;
	}

	if (Info->RecvWorker)
	{
		delete Info->RecvWorker;
	}

	if (Info->Socket)
	{
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(Info->Socket);
	}

	delete Info;

	UE_LOG(LogTemp, Warning, TEXT("Client %d disconnected."), ClientId);

	OnClientDisconnected.Broadcast(ClientId);
}

bool UTCPServerSubsystem::SendToClient(int32 ClientId, const uint8* Body, uint32 BodyLength)
{
	FTCPClientInfo* Info = nullptr;

	{
		FScopeLock Lock(&ClientsLock);
		FTCPClientInfo** Found = Clients.Find(ClientId);
		if (!Found)
		{
			return false;
		}
		Info = *Found;
	}

	if (!Info->Socket)
	{
		return false;
	}

	//2바이트 길이 헤더 + Body (TCPClientSubsystem::SendAll과 동일한 프레이밍)
	TArray<uint8> Packet;
	Packet.Reserve(2 + BodyLength);
	Packet.Add((uint8)((BodyLength >> 8) & 0xFF));
	Packet.Add((uint8)((BodyLength) & 0xFF));
	Packet.Append(Body, BodyLength);

	int32 SentTotalBytes = 0;
	while (SentTotalBytes < Packet.Num())
	{
		int32 SentBytes = 0;
		if (!Info->Socket->Send(Packet.GetData() + SentTotalBytes, Packet.Num() - SentTotalBytes, SentBytes) || SentBytes < 0)
		{
			return false;
		}
		SentTotalBytes += SentBytes;
	}

	return true;
}

void UTCPServerSubsystem::BroadcastChat(const FString& UserId, const FString& Message)
{
	flatbuffers::FlatBufferBuilder Builder(256);

	auto UserIdOffset = Builder.CreateString(TCHAR_TO_UTF8(*UserId));
	auto MessageOffset = Builder.CreateString(TCHAR_TO_UTF8(*Message));

	auto ChatOffset = UserPacket::CreateS2C_Chat(Builder, 0, UserIdOffset, MessageOffset);
	auto PacketOffset = UserPacket::CreatePacketData(Builder, UserPacket::PacketType_S2C_Chat, ChatOffset.Union());

	UserPacket::FinishPacketDataBuffer(Builder, PacketOffset);

	TArray<int32> ClientIds;
	{
		FScopeLock Lock(&ClientsLock);
		Clients.GetKeys(ClientIds);
	}

	for (int32 ClientId : ClientIds)
	{
		SendToClient(ClientId, Builder.GetBufferPointer(), Builder.GetSize());
	}
}

void UTCPServerSubsystem::SendLocalChat(const FString& UserId, const FString& Message)
{
	// 호스트 자신은 소켓을 거칠 필요가 없다 : 다른 클라이언트에게만 뿌리고, 자기 UI는 델리게이트로 바로 갱신
	BroadcastChat(UserId, Message);
	OnChatReceived.Broadcast(-1, UserId, Message); // ClientId -1 : 호스트 자신을 의미
}

void UTCPServerSubsystem::DispatchPacket(int32 ClientId, const TArray<uint8>& Buffer)
{
	if (Buffer.Num() == 0)
	{
		return;
	}

	flatbuffers::Verifier Verifier(Buffer.GetData(), Buffer.Num());
	if (!UserPacket::VerifyPacketDataBuffer(Verifier))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid packet from client %d, dropping."), ClientId);
		return;
	}

	const auto* UserPacketData = UserPacket::GetPacketData(Buffer.GetData());

	switch (UserPacketData->data_type())
	{
	case UserPacket::PacketType_C2S_Chat:
	{
		const auto* ChatData = UserPacketData->data_as_C2S_Chat();
		if (ChatData && ChatData->user_id() && ChatData->message())
		{
			FString UserId = UTF8_TO_TCHAR(ChatData->user_id()->c_str());
			FString Message = UTF8_TO_TCHAR(ChatData->message()->c_str());

			OnChatReceived.Broadcast(ClientId, UserId, Message);

			// 받은 채팅을 접속한 모든 클라이언트에게 다시 뿌린다
			BroadcastChat(UserId, Message);
		}
	}
	break;

	// TODO: PacketType_C2S_Login, PacketType_C2S_Signup 등 다른 케이스도 여기 추가

	default:
		break;
	}
}

TStatId UTCPServerSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTCPServerSubsystem, STATGROUP_Tickables);
}

void UTCPServerSubsystem::Tick(float DeltaTime)
{
	// 1) Accept 스레드에서 등록된 신규 접속을 게임 스레드에서 안전하게 알림
	int32 ConnectedId = 0;
	while (PendingConnectedIds.Dequeue(ConnectedId))
	{
		OnClientConnected.Broadcast(ConnectedId);
	}

	// 2) 각 클라이언트의 수신 큐를 비우며 패킷 처리
	TArray<int32> ClientIds;
	{
		FScopeLock Lock(&ClientsLock);
		Clients.GetKeys(ClientIds);
	}

	for (int32 ClientId : ClientIds)
	{
		FTCPClientInfo* Info = nullptr;
		{
			FScopeLock Lock(&ClientsLock);
			FTCPClientInfo** Found = Clients.Find(ClientId);
			if (!Found)
			{
				continue;
			}
			Info = *Found;
		}

		// 연결이 끊긴 소켓이면 정리
		if (!Info->Socket || Info->Socket->GetConnectionState() != SCS_Connected)
		{
			DisconnectClient(ClientId);
			continue;
		}

		TArray<uint8> Packet;
		while (Info->RecvQueue.Dequeue(Packet))
		{
			DispatchPacket(ClientId, Packet);
		}
	}
}