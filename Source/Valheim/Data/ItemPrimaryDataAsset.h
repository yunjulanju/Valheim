// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Data/ItemDataStruct.h"
#include "ItemPrimaryDataAsset.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class VALHEIM_API UItemPrimaryDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FName ItemID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FItemTextData TextData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FItemCategory ItemCategory;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FItemAssetData AssetData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item")
	FItemNumericData NumericData;


	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("Item"), ItemID);
	}
};
