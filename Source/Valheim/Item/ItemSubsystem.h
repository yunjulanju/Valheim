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

	// ItemID로 정적 아이템 데이터 조회
	UFUNCTION(BlueprintCallable, Category = "Item")
	bool GetItemData(FName ItemID, UItemPrimaryDataAsset*& OutData) const;

	// 존재 여부만 확인
	UFUNCTION(BlueprintCallable, Category = "Item")
	bool DoesItemExist(FName ItemID) const;

protected:
	// 모든 아이템 애셋 목록을 담은 레지스트리 (GameInstance에서 받아옴)
	UPROPERTY()
	TObjectPtr<UItemRegistry> ItemRegistry;

private:
	// 빠른 조회용 캐시: ItemID → 실제 애셋
	TMap<FName, TObjectPtr<UItemPrimaryDataAsset>> ItemDataCache;

	void BuildItemCache();
};
