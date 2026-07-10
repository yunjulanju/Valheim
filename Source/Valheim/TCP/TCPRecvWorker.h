#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"
#include "Containers/Queue.h"

#include <atomic>


class FSOCKET;

class VALHEIM_API FTCPRecvWorker : public FRunnable
{
public:
	FTCPRecvWorker(FSocket* InServerSocket, TQueue<TArray<uint8>>& InRecvQueue);

	uint32 Run() override;

	virtual void Stop() override;


private:
	TArray<uint8> RecvBuffer;
	FSocket* ServerSocket = nullptr;

	TQueue<TArray<uint8>>& RecvQueue;

	std::atomic<bool> bStopRequested = false;

};