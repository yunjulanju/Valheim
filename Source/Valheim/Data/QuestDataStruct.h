#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "QuestDataStruct.generated.h" 

UENUM(BlueprintType)
enum class EQuestType : uint8
{
    Move    UMETA(DisplayName = "Move"),
    Kill    UMETA(DisplayName = "Kill"),
    UseItem UMETA(DisplayName = "UseItem"),
    CollectItem UMETA(DisplayName = "CollectItem")
};

UENUM(BlueprintType)
enum class EQuestStatus : uint8
{
    InProgress       UMETA(DisplayName = "InProgress"),
    ReadyToComplete  UMETA(DisplayName = "ReadyToComplete"),
    Completed        UMETA(DisplayName = "Completed")
};

USTRUCT(BlueprintType)
struct FQuestRewardItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
    FName RewardItemID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
    int32 RewardItemAmount = 0;
};

USTRUCT(BlueprintType)
struct FQuestData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName NPCID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EQuestType Type;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName TargetID; //몬스터 이름, 아이템 이름으로

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 RequiredAmount = 1;

    // 보상
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Reward")
    TArray<FQuestRewardItem> RewardItems;
};

USTRUCT(BlueprintType)
struct FActiveQuest
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 CurrentAmount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EQuestStatus Status = EQuestStatus::InProgress;
};