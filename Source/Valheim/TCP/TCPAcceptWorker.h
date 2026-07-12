#pragma once
#include "CoreMinimal.h"
#include "HAL/Runnable.h"

#include <atomic>

class FSocket;
class UTCPServerSubsystem;

class VALHEIM_API FTCPAcceptWorker : public FRunnable
{
public:
	FTCPAcceptWorker(FSocket* InListenSocket, UTCPServerSubsystem* InServer);

	uint32 Run() override;

	virtual void Stop() override;

private:
	FSocket* ListenSocket = nullptr;
	UTCPServerSubsystem* Server = nullptr;

	std::atomic<bool> bStopRequested = false;
};