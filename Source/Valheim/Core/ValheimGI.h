// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ValheimGI.generated.h"

/**
 * 
 */

class UItemRegistry;
UCLASS()
class VALHEIM_API UValheimGI : public UGameInstance
{
	GENERATED_BODY()

public:

	virtual void Init() override;

	UPROPERTY(EditDefaultsOnly, Category = "TCP")
	FString ServerHost = TEXT("127.0.0.1");

	UPROPERTY(EditDefaultsOnly, Category = "TCP")
	int32 ServerPort = 35000;

	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	UDataTable* QuestDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Item")
	UItemRegistry* ItemRegistry;

	UPROPERTY(BlueprintReadWrite, Category = "Network")
	FString LevelName;
};
