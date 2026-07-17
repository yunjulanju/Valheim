// Fill out your copyright notice in the Description page of Project Settings.


#include "TCP/TCPClientSubsystem.h"
#include "SocketSubsystem.h"
#include "Sockets.h"
#include "IPAddress.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "TCPRecvWorker.h"
#include "HAL/RunnableThread.h"

#include "UserPacket_generated.h"

void UTCPClientSubsystem::Deinitialize()
{
	Disconnect();

	Super::Deinitialize();
}

bool UTCPClientSubsystem::Connect(const FString& Host, int32 Port)
{
	//WSStartup
	ISocketSubsystem* SocketSubSystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

	//socket
	//SOCKADDR

	//hostent
	//getaddrinfo()
	FAddressInfoResult AddrInfo = SocketSubSystem->GetAddressInfo(*Host, nullptr, EAddressInfoFlags::Default, NAME_None);

	//SOCKADDR(IP)
	TSharedRef<FInternetAddr> ServerAddr = AddrInfo.Results[0].Address;

	//Port
	ServerAddr->SetPort(Port);

	//socket()
	ServerSocket = SocketSubSystem->CreateSocket(NAME_Stream, TEXT("TCPClient"), ServerAddr->GetProtocolType());

	//connect()
	if (!ServerSocket->Connect(*ServerAddr))
	{
		UE_LOG(LogTemp, Warning, TEXT("Connect Failed."));
		return false;
	}

	ServerSocket->SetNonBlocking(false);

	//UE_LOG(LogTemp, Warning, TEXT("TCP Connected to %s:%d"), *Host, Port);
	RecvQueue.Empty();

	RecvWorker = new FTCPRecvWorker(ServerSocket, RecvQueue);
	RecvThread = FRunnableThread::Create(RecvWorker, TEXT("TCPRecvWoker"));


	OnTCPConnected.Broadcast();

	return true;
}

void UTCPClientSubsystem::Disconnect()
{
	if (RecvWorker)
	{
		RecvWorker->Stop();
	}

	if (ServerSocket)
	{
		ServerSocket->Close();
	}

	if (RecvThread)
	{
		RecvThread->Kill(true);
		delete RecvThread;
		RecvThread = nullptr;
	}

	if (RecvWorker)
	{
		delete RecvWorker;
		RecvWorker = nullptr;
	}

	RecvQueue.Empty();
	OnTCPDisconnected.Broadcast();

	if (ServerSocket)
	{
		ISocketSubsystem* SocketSubSystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);

		//closesocket
		ServerSocket->Close();
		SocketSubSystem->DestroySocket(ServerSocket);

		ServerSocket = nullptr;
	}
}

bool UTCPClientSubsystem::IsConncted() const
{
	return ServerSocket != nullptr && ServerSocket->GetConnectionState() == SCS_Connected;
}

void UTCPClientSubsystem::RecvAll()
{
}

bool UTCPClientSubsystem::SendAll(const uint8* Body, uint32 BodyLength)
{
	//char Buffer[2 + BodyLength];
	TArray<uint8> Packet;

	Packet.Reserve(2 + BodyLength);
	Packet.Add((uint8)((BodyLength >> 8) & 0xFF));
	Packet.Add((uint8)((BodyLength) & 0xFF));
	Packet.Append(Body, BodyLength);

	int32 SentTotalBytes = 0;
	while (SentTotalBytes < Packet.Num())
	{
		int32 SentBytes = 0;
		if (!ServerSocket->Send(Packet.GetData() + SentTotalBytes, Packet.Num() - SentTotalBytes, SentBytes) || SentBytes < 0)
		{
			return false;
		}
		SentTotalBytes += SentBytes;
	}

	return true;
}

bool UTCPClientSubsystem::SendChat(const FString& UserId, const FString& Message)
{
	if (!IsConncted())
	{
		UE_LOG(LogTemp, Warning, TEXT("SendChat failed: not connected."));
		return false;
	}

	flatbuffers::FlatBufferBuilder Builder;

	auto ChatData = UserPacket::CreateC2S_ChatDirect(
		Builder,
		0,
		TCHAR_TO_UTF8(*UserId),
		TCHAR_TO_UTF8(*Message)
	);

	auto PacketData = UserPacket::CreatePacketData(
		Builder,
		UserPacket::PacketType_C2S_Chat,
		ChatData.Union()
	);

	Builder.Finish(PacketData);

	bool bSent = SendAll(Builder.GetBufferPointer(), Builder.GetSize());
	//UE_LOG(LogTemp, Warning, TEXT("SendChat [%s]: %s -> %s"), *UserId, *Message, bSent ? TEXT("OK") : TEXT("FAILED"));
	return bSent;
}

void UTCPClientSubsystem::DispatchPacket()
{
	if (RecvBuffer.Num() == 0)
	{
		return;
	}

	flatbuffers::Verifier Verifier(RecvBuffer.GetData(), RecvBuffer.Num());
	if (!UserPacket::VerifyPacketDataBuffer(Verifier))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid packet received, dropping."));
		return;
	}

	const auto UserPacketData = UserPacket::GetPacketData(RecvBuffer.GetData());

	//UE_LOG(LogTemp, Warning, TEXT("DispatchPacket: received data_type=%d"), (int32)UserPacketData->data_type());

	switch (UserPacketData->data_type())
	{
	case UserPacket::PacketType_S2C_Chat:
	{
		const auto* ChatData = UserPacketData->data_as_S2C_Chat();
		if (ChatData)
		{
			FString UserId = UTF8_TO_TCHAR(ChatData->user_id()->c_str());
			FString Message = UTF8_TO_TCHAR(ChatData->message()->c_str());
			//UE_LOG(LogTemp, Warning, TEXT("DispatchPacket: S2C_Chat received [%s]: %s"), *UserId, *Message);
			OnChatReceived.Broadcast(UserId, Message);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("DispatchPacket: S2C_Chat data_as_S2C_Chat() returned null"));
		}
	}
	break;
	case UserPacket::PacketType_S2C_Login:
	{
		/*const auto* LoginData = UserPacketData->data_as_S2C_Login();

		FString Message = UTF8_TO_TCHAR(LoginData->message()->c_str());

		UE_LOG(LogTemp, Warning, TEXT("Login %d %s"), LoginData->client_socket_id(), *Message);

		OnLogin.Broadcast(LoginData->success(), Message);*/
	}
	break;
	case UserPacket::PacketType_S2C_Spawn:
	{
	}
	break;
	case UserPacket::PacketType_S2C_Move:
	{

	}
	break;
	case UserPacket::PacketType_S2C_Destroy:
	{
	}
	break;
	case UserPacket::PacketType_S2C_ChangeColor:
	{
	}
	break;
	}
}

TStatId UTCPClientSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UTCPClientSubsystem, STATGROUP_Tickables);
}

void UTCPClientSubsystem::Tick(float DeltaTime)
{
	if (!RecvWorker)
	{
		return;
	}

	TArray<uint8> Packet;
	while (RecvQueue.Dequeue(Packet))
	{
		RecvBuffer = MoveTemp(Packet);
		DispatchPacket();
	}
}