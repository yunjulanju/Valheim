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
struct FQuestData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FName QuestID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    EQuestType Type;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 RequiredAmount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FVector TargetLocation = FVector::ZeroVector;

    // º¸»ó
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    int32 RewardExp = 0;
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