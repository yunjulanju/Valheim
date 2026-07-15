// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/ValheimGI.h"
#include "TCP/TCPClientSubsystem.h"

void UValheimGI::Init()
{
	Super::Init();

	if (UTCPClientSubsystem* TCPSubsystem = GetSubsystem<UTCPClientSubsystem>())
	{
		bool bConnected = TCPSubsystem->Connect(ServerHost, ServerPort);

		if (!bConnected)
		{
			UE_LOG(LogTemp, Warning, TEXT("ValheimGameInstance::Init - TCP Connect failed. Host=%s Port=%d"), *ServerHost, ServerPort);
		}
	}
}
