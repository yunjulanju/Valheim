// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Data/ItemDataStruct.h"
#include "ItemDataBase.generated.h"

/**
 * 
 */
class UInventoryComponent;
UCLASS()
class VALHEIM_API UItemDataBase : public UObject
{
	GENERATED_BODY()
public:
    //------------------------------Function ////
    UItemDataBase();

    UItemDataBase* CreateItemCopy() const;

    UFUNCTION()
    FORCEINLINE bool IsStackable() const { return NumericData.bIsStackable; };

    UFUNCTION()
    FORCEINLINE bool IsFullStack() const { return  Quantity == NumericData.MaxStackSize; };

    UFUNCTION()
	void SetQuantity(const int32 NewQuantity);

    void ResetItemFlags();

    //Use Funtion? - Interface function

    //------------------------------Property ////

//Data

    UPROPERTY()
    UInventoryComponent* OwningInventory;

    bool bIsCopy;
    bool bIsPickup;
    //Item

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    FName  ItemID;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    FItemTextData TextData;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    EItemCategory ItemCategory;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    FItemAssetData AssetData;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    FItemNumericData NumericData;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
    int32 Quantity = 1;

protected:

    bool operator ==(const FName& OtherID) const
    {
        return this->ItemID == OtherID;
    }
	
};
