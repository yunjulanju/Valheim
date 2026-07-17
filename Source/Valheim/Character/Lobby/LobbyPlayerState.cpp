// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Lobby/LobbyPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Core/ValheimGI.h"

void ALobbyPlayerState::SetNickname(const FText& NewNickname)
{
	if (HasAuthority())
	{
		Nickname = NewNickname;

		SaveNicknameToGameInstance();
	}
}

void ALobbyPlayerState::OnRep_Nickname()
{
	// Nickname 이 변경되었을떄 호출
	SaveNicknameToGameInstance();
}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, Nickname);
}

void ALobbyPlayerState::SaveNicknameToGameInstance()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || !PC->IsLocalController())
	{
		return;
	}

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		return;
	}

	 UValheimGI* ValheimGI = Cast<UValheimGI>(GI);
	 if (ValheimGI)
	 {
	     ValheimGI->SavedNickname = Nickname.ToString();
	 }
}
