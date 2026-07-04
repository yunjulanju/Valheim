// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ItemRegistry.generated.h"

class UItemPrimaryDataAsset;
UCLASS()
class VALHEIM_API UItemRegistry : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Item")
	TArray<TObjectPtr<UItemPrimaryDataAsset>> AllItems;
};
