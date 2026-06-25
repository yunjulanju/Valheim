// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestSubsystem.h"

void UQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}

bool UQuestSubsystem::GetQuestData(FName QuestID, FQuestData& OutData) const
{
	return false;
}

bool UQuestSubsystem::DoesQuestExist(FName QuestID) const
{
	return false;
}

void UQuestSubsystem::BuildQuestCache()
{
}
