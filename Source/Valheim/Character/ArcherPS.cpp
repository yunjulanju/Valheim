// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ArcherPS.h"
#include "Net/UnrealNetwork.h"
#include "Quest/QuestSubsystem.h"
#include "Engine/GameInstance.h"



AArcherPS::AArcherPS()
{
}

void AArcherPS::BeginPlay()
{
	Super::BeginPlay();

	if (GetGameInstance())
	{
		QuestSubsystem = GetGameInstance()->GetSubsystem<UQuestSubsystem>();
	}
}


bool AArcherPS::AcceptQuest(FName QuestID)
{
	UE_LOG(LogTemp, Warning, TEXT("AArcherPS::AcceptQuest(FName QuestID)"))
	if (!HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcherPS::AcceptQuest(FName QuestID) !HasAuthority()"))
		return false;
	}
	if (IsQuestCompleted(QuestID))
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcherPS::AcceptQuest(FName QuestID) IsQuestCompleted(QuestID)"))
		return false;
	}
	if (!QuestSubsystem || !QuestSubsystem->DoesQuestExist(QuestID))
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcherPS::AcceptQuest(FName QuestID) !QuestSubsystem"))
		return false;
	}

	FActiveQuest ExistingQuest;
	if (GetActiveQuest(QuestID, ExistingQuest))
	{
		UE_LOG(LogTemp, Warning, TEXT("AArcherPS::AcceptQuest(FName QuestID) GetActiveQuest(QuestID, ExistingQuest)"))
		return false;
	}

	FActiveQuest NewQuest;
	NewQuest.QuestID = QuestID;
	NewQuest.CurrentAmount = 0;
	NewQuest.Status = EQuestStatus::InProgress;
	ActiveQuests.Add(NewQuest);

	OnActiveQuestsChanged.Broadcast();
	return true;
}

bool AArcherPS::AbandonQuest(FName QuestID)
{
	if (!HasAuthority())
	{
		return false;
	}

	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		if (ActiveQuests[i].QuestID == QuestID)
		{
			ActiveQuests.RemoveAt(i);
			OnActiveQuestsChanged.Broadcast();
			return true;
		}
	}

	return false;
}

bool AArcherPS::CompleteQuest(FName QuestID)
{
	if (!HasAuthority())
	{
		return false;
	}

	for (int32 i = 0; i < ActiveQuests.Num(); ++i)
	{
		if (ActiveQuests[i].QuestID == QuestID)
		{
			if (ActiveQuests[i].Status != EQuestStatus::ReadyToComplete)
			{
				return false;
			}

			CompletedQuestIDs.Add(QuestID);
			ActiveQuests.RemoveAt(i);

			OnActiveQuestsChanged.Broadcast();
			return true;
		}
	}
	return false;
}

bool AArcherPS::GetActiveQuest(FName QuestID, FActiveQuest& OutActiveQuest) const
{
	for (const FActiveQuest& Quest : ActiveQuests)
	{
		if (Quest.QuestID == QuestID)
		{
			OutActiveQuest = Quest;
			return true;
		}
	}
	return false;
}

void AArcherPS::UpdateQuestProgressByEvent(EQuestType Type, FName TargetID, int32 Amount)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!QuestSubsystem)
	{
		return;
	}

	for (FActiveQuest& Quest : ActiveQuests)
	{
		if (Quest.Status != EQuestStatus::InProgress)
		{
			continue;
		}

		FQuestData QuestData;
		if (!QuestSubsystem->GetQuestData(Quest.QuestID, QuestData))
		{
			continue;
		}

		if (QuestData.Type != Type || QuestData.TargetID != TargetID)
		{
			continue;
		}

		//여긴 아직 퀘스트 타입에 따라 다름
		Quest.CurrentAmount = FMath::Clamp(Quest.CurrentAmount + Amount, 0, QuestData.RequiredAmount);

		if (Quest.CurrentAmount >= QuestData.RequiredAmount)
		{
			Quest.Status = EQuestStatus::ReadyToComplete;
		}

		OnActiveQuestsChanged.Broadcast();
	}
}

void AArcherPS::OnRep_ActiveQuests()
{
	OnActiveQuestsChanged.Broadcast();
}

void AArcherPS::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AArcherPS, ActiveQuests);
	DOREPLIFETIME(AArcherPS, CompletedQuestIDs);
}