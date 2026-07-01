// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ValheimGI.generated.h"

/**
 * 
 */
UCLASS()
class VALHEIM_API UValheimGI : public UGameInstance
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, Category = "Quest")
	UDataTable* QuestDataTable;
	
};
