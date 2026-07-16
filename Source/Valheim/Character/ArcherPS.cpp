// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ArcherPS.h"
#include "Net/UnrealNetwork.h"
#include "Quest/QuestSubsystem.h"
#include "Engine/GameInstance.h"
#include "Archer.h"
#include <Inventory/InventoryComponent.h>
#include "Item/ItemSubsystem.h"



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
	
	ServerAcceptQuest(QuestID);
	return true;
}

void AArcherPS::ServerAcceptQuest_Implementation(FName QuestID)
{
	FActiveQuest NewQuest;
	NewQuest.QuestID = QuestID;
	NewQuest.CurrentAmount = 0;
	NewQuest.Status = EQuestStatus::InProgress;
	ActiveQuests.Add(NewQuest);

	OnActiveQuestsChanged.Broadcast();
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
			ServerAbandonQuest(i);
			return true;
		}
	}

	return false;
}

void AArcherPS::ServerAbandonQuest_Implementation(int Index)
{
	ActiveQuests.RemoveAt(Index);
	OnActiveQuestsChanged.Broadcast();
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

			ServerCompleteQuest(QuestID, i);
			return true;
		}
	}
	return false;
}

void AArcherPS::ServerCompleteQuest_Implementation(FName QuestID, int Index)
{
	if (!QuestSubsystem)
	{
		UE_LOG(LogTemp, Warning, TEXT("CompleteQuest Failed: QuestSubsystem is NULL"));
		return;
	}

	FQuestData QuestData;
	if (!QuestSubsystem->GetQuestData(QuestID, QuestData))
	{
		UE_LOG(LogTemp, Warning, TEXT("CompleteQuest Failed: Failed to find QuestData for ID '%s'"), *QuestID.ToString());
		return;
	}

	AArcher* Archer = Cast<AArcher>(GetPawn());
	if (!Archer)
	{
		UE_LOG(LogTemp, Warning, TEXT("CompleteQuest Failed: Pawn is not AArcher"));
		return;
	}

	UInventoryComponent* Inv = Archer->GetInventory();
	if (!Inv)
	{
		UE_LOG(LogTemp, Warning, TEXT("CompleteQuest Failed: InventoryComponent is NULL on Archer"));
		return;
	}

	UItemSubsystem* ItemSub = GetGameInstance()->GetSubsystem<UItemSubsystem>();
	if (!ItemSub)
	{
		UE_LOG(LogTemp, Warning, TEXT("CompleteQuest Failed: ItemSubsystem is NULL"));
		return;
	}

	for (const FQuestRewardItem& Reward : QuestData.RewardItems)
	{
		if (Reward.RewardItemID.IsNone() || Reward.RewardItemAmount <= 0)
		{
			continue;
		}

		if (!ItemSub->DoesItemExist(Reward.RewardItemID))
		{
			UE_LOG(LogTemp, Warning, TEXT("Quest reward ItemID '%s' not registered in ItemRegistry"), *Reward.RewardItemID.ToString());
		}

		FItemAddResult Result = Inv->HandleAddItem(Reward.RewardItemID, Reward.RewardItemAmount);

		if (Result.OperationResult == EItemAddResult::NoItemAdded)
		{
			UE_LOG(LogTemp, Warning, TEXT("Quest reward item '%s' could not be added, inventory full"), *Reward.RewardItemID.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully added quest reward: %s x%d"), *Reward.RewardItemID.ToString(), Reward.RewardItemAmount);
		}
	}

	CompletedQuestIDs.Add(QuestID);
	ActiveQuests.RemoveAt(Index);

	OnActiveQuestsChanged.Broadcast();
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
		ServerUpdateQuestProgressByEvent(Type, TargetID, Amount);
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

void AArcherPS::ServerUpdateQuestProgressByEvent_Implementation(EQuestType Type, FName TargetID, int32 Amount)
{
	UpdateQuestProgressByEvent(Type, TargetID, Amount);
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