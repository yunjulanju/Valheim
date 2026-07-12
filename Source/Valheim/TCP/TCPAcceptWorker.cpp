#include "TCPAcceptWorker.h"
#include "TCPServerSubsystem.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "HAL/PlatformProcess.h"

FTCPAcceptWorker::FTCPAcceptWorker(FSocket* InListenSocket, UTCPServerSubsystem* InServer) :
	ListenSocket(InListenSocket), Server(InServer)
{
}

uint32 FTCPAcceptWorker::Run()
{
	while (!bStopRequested && ListenSocket)
	{
		bool bHasPendingConnection = false;

		if (!ListenSocket->HasPendingConnection(bHasPendingConnection) || !bHasPendingConnection)
		{
			FPlatformProcess::Sleep(0.01f);
			continue;
		}

		//accept() : 货 努扼捞攫飘 家南 积己
		FSocket* NewClientSocket = ListenSocket->Accept(TEXT("TCPServerClient"));

		if (NewClientSocket && Server)
		{
			Server->RegisterClient(NewClientSocket);
		}
	}

	return 0;
}

void FTCPAcceptWorker::Stop()
{
	bStopRequested = true;
}