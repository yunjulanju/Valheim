// Fill out your copyright notice in the Description page of Project Settings.


#include "NPC/NPC.h"
#include "Quest/QuestSubsystem.h"
#include "Character/ArcherPS.h"

ANPC::ANPC()
{
	PrimaryActorTick.bCanEverTick = true;

}

void ANPC::BeginPlay()
{
	Super::BeginPlay();

	if (GetGameInstance())
	{
		QuestSubsystem = GetGameInstance()->GetSubsystem<UQuestSubsystem>();
	}

	CachedQuestIDs = QuestSubsystem->GetQuestsByGiver(NPCID);
	
}

void ANPC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ANPC::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ANPC::Interact(APawn* Interactor)
{
	if (!Interactor)
	{
		return;
	}

	AArcherPS* QuestPlayerState = Interactor->GetPlayerState<AArcherPS>();
	if (!QuestPlayerState)
	{
		return;
	}

	HandleQuestInteraction(QuestPlayerState);
}

void ANPC::HandleQuestInteraction(AArcherPS* QuestPlayerState)
{

	if (!QuestPlayerState)
	{
		return;
	}

	for (const FName& QuestID : CachedQuestIDs)
	{
		if (QuestPlayerState->IsQuestCompleted(QuestID))
		{
			continue;
		}

		FActiveQuest ActiveQuest;
		if (QuestPlayerState->GetActiveQuest(QuestID, ActiveQuest))
		{
			if (ActiveQuest.Status == EQuestStatus::ReadyToComplete)
			{
				QuestPlayerState->CompleteQuest(QuestID);
			}
			continue;
		}

		QuestPlayerState->AcceptQuest(QuestID);
		return;
	}
}