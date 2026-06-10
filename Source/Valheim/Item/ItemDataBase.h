// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Data/ItemDataStruct.h"
#include "ItemDataBase.generated.h"

/**
 * 
 */
UCLASS()
class VALHEIM_API UItemDataBase : public UObject
{
	GENERATED_BODY()
public:
    //------------------------------Function ////

	FORCEINLINE bool IsStackable() const { return ItemData && ItemData->NumericData.bIsStackable; };

	FORCEINLINE bool IsFullStack() const { return ItemData && Quantity == ItemData->NumericData.MaxStackSize; };

	void SetQuantity(const int32 NewQuantity);

    //------------------------------Property ////

//Data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDataTableRowHandle ItemDataRowHandle;
    FItemBaseRow* ItemData = nullptr;

    //Item
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    int32 Quantity = 1;
	
};
