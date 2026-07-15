// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ArcherPC.h"
#include "TCP/TCPClientSubSystem.h"

AArcherPC::AArcherPC()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AArcherPC::BeginPlay()
{
	Super::BeginPlay();

	const FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	if (!IsLocalController())
	{
		return; // 서버가 들고 있는 원격 클라이언트용 PC 인스턴스는 여기서 끝
	}
}

void AArcherPC::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UTCPClientSubsystem* TCP = GetTCP();

	TCP->OnTCPConnected.RemoveDynamic(this, &AArcherPC::HandleConnected);
	TCP->OnTCPDisconnected.RemoveDynamic(this, &AArcherPC::HandleDisconnect);

	Super::EndPlay(EndPlayReason);
}

void AArcherPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

UTCPClientSubsystem* AArcherPC::GetTCP()
{
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		return GI->GetSubsystem<UTCPClientSubsystem>();
	}

	return nullptr;
}

void AArcherPC::HandleConnected()
{
	UE_LOG(LogTemp, Warning, TEXT("Connected Server"));

	UTCPClientSubsystem* TCP = GetTCP();

	//TCP->SendLogin(TEXT("junios"), TEXT("1234"));
}

void AArcherPC::HandleDisconnect()
{
	UE_LOG(LogTemp, Warning, TEXT("Disonnected Server"));
}
