// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Lobby/Lobby.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Character/Lobby/LobbyPlayerState.h"
#include "GameFramework/GameState.h"

TArray<ALobbyPlayerState*> ULobby::GetLobbyPlayerStates()
{
	TArray<ALobbyPlayerState*> LobbyPlayerStates;

	if (AGameStateBase* GameState = UGameplayStatics::GetGameState(this))
	{
		for (APlayerState* PlayerState : GameState->PlayerArray)
		{
			ALobbyPlayerState* LobbyPlayerState = Cast<ALobbyPlayerState>(PlayerState);
			if (LobbyPlayerState == nullptr)
				continue;

			LobbyPlayerStates.Add(LobbyPlayerState);
		}
	}

	return LobbyPlayerStates;
}

ALobbyPlayerState* ULobby::GetLobbyPlayerStateAtIndex(int32 InIndex)
{
	TArray<ALobbyPlayerState*> LobbyPlayerState = GetLobbyPlayerStates();

	if (InIndex < LobbyPlayerState.Num())
		return LobbyPlayerState[InIndex];

	return nullptr;
}
