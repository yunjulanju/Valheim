// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Lobby/LobbyPlayerState.h"
#include "Net/UnrealNetwork.h"

void ALobbyPlayerState::OnRep_Nickname()
{
	// Nickname 이 변경되었을떄 호출

}

void ALobbyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPlayerState, Nickname);
}
