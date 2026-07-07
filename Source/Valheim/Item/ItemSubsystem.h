// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemSubsystem.generated.h"

class UItemPrimaryDataAsset;
class UItemRegistry;
UCLASS()
class VALHEIM_API UItemSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool GetItemData(FName ItemID, UItemPrimaryDataAsset*& OutData) const;

	UFUNCTION(BlueprintCallable, Category = "Item")
	bool DoesItemExist(FName ItemID) const;

protected:
	UPROPERTY()
	TObjectPtr<UItemRegistry> ItemRegistry;

private:
	TMap<FName, TObjectPtr<UItemPrimaryDataAsset>> ItemDataCache;

	void BuildItemCache();
};
