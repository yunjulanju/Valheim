// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Data/QuestDataStruct.h"
#include "QuestSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class VALHEIM_API UQuestSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool GetQuestData(FName QuestID, FQuestData& OutData) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    TArray<FName> GetQuestsByGiver(FName NPCID) const;

    UFUNCTION(BlueprintCallable, Category = "Quest")
    bool DoesQuestExist(FName QuestID) const;

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Quest")
    UDataTable* QuestDataTable;

private:
    TMap<FName, FQuestData*> QuestDataCache;
    TMap<FName, TArray<FName>> QuestsByGiverCache;

    void BuildQuestCache();
};
