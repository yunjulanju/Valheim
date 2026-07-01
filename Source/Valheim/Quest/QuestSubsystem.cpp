// Fill out your copyright notice in the Description page of Project Settings.


#include "Quest/QuestSubsystem.h"
#include "Core/ValheimGI.h"

void UQuestSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

    if (const UValheimGI* MyGameInstance = Cast<UValheimGI>(GetGameInstance()))
    {
        QuestDataTable = MyGameInstance->QuestDataTable;
    }

    // 퀘스트 자료 캐싱
    BuildQuestCache();
}

bool UQuestSubsystem::GetQuestData(FName QuestID, FQuestData& OutData) const
{
    //포인터를 가리키는 포인터 값을 가져와야해서 이중 포인터가 쓰임
    if (const FQuestData* const* FoundRow = QuestDataCache.Find(QuestID))
    {
        OutData = **FoundRow;
        return true;
    }
	return false;
}

TArray<FName> UQuestSubsystem::GetQuestsByGiver(FName NPCID) const
{
    if (const TArray<FName>* Found = QuestsByGiverCache.Find(NPCID))
    {
        return *Found;
    }
    return TArray<FName>();
}

bool UQuestSubsystem::DoesQuestExist(FName QuestID) const
{
    return QuestDataCache.Contains(QuestID);
}

void UQuestSubsystem::BuildQuestCache()
{
    QuestDataCache.Empty();
    QuestsByGiverCache.Empty();

    if (!QuestDataTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("QuestSubsystem: QuestDataTable NULL"));
        return;
    }
    static const FString ContextString(TEXT("BuildQuestCache"));
    TArray<FName> RowNames = QuestDataTable->GetRowNames();

    for (const FName& RowName : RowNames)
    {
        if (FQuestData* Row = QuestDataTable->FindRow<FQuestData>(RowName, ContextString))
        {
            QuestDataCache.Add(RowName, Row);
            QuestsByGiverCache.FindOrAdd(Row->NPCID).Add(RowName);
        }
    }

}
