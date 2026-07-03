// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/QuestDataStruct.h"
#include "GameFramework/PlayerState.h"
#include "ArcherPS.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActiveQuestsChanged);

class UQuestSubsystem;
UCLASS()
class VALHEIM_API AArcherPS : public APlayerState
{
	GENERATED_BODY()

public:
	AArcherPS();

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool AcceptQuest(FName QuestID);

	UFUNCTION(Server, Reliable)
	void ServerAcceptQuest(FName QuestID);
	void ServerAcceptQuest_Implementation(FName QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool AbandonQuest(FName QuestID);
	UFUNCTION(Server, Reliable)
	void ServerAbandonQuest(int Index);
	void ServerAbandonQuest_Implementation(int Index);


	UFUNCTION(BlueprintCallable, Category = "Quest")
	void UpdateQuestProgressByEvent(EQuestType Type, FName TargetID, int32 Amount);
	UFUNCTION(Server, Reliable)
	void ServerUpdateQuestProgressByEvent(EQuestType Type, FName TargetID, int32 Amount);
	void ServerUpdateQuestProgressByEvent_Implementation(EQuestType Type, FName TargetID, int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool CompleteQuest(FName QuestID);
	UFUNCTION(Server, Reliable)
	void ServerCompleteQuest(FName QuestID, int Index);
	void ServerCompleteQuest_Implementation(FName QuestID, int Index);

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool IsQuestCompleted(FName QuestID) const { return CompletedQuestIDs.Contains(QuestID); }

	UFUNCTION(BlueprintCallable, Category = "Quest")
	bool GetActiveQuest(FName QuestID, FActiveQuest& OutActiveQuest) const;

	UFUNCTION(BlueprintCallable, Category = "Quest")
	const TArray<FActiveQuest>& GetActiveQuests() const { return ActiveQuests; }
	

	UPROPERTY(BlueprintAssignable, Category = "Quest")
	FOnActiveQuestsChanged OnActiveQuestsChanged;

protected:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_ActiveQuests();

private:
	UPROPERTY(ReplicatedUsing = OnRep_ActiveQuests)
	TArray<FActiveQuest> ActiveQuests;

	UPROPERTY(Replicated)
	TArray<FName> CompletedQuestIDs;

	const UQuestSubsystem* QuestSubsystem;
};
