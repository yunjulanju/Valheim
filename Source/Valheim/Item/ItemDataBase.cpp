// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemDataBase.h"

UItemDataBase::UItemDataBase()
{
}

UItemDataBase* UItemDataBase::CreateItemCopy() const
{
	UItemDataBase* ItemCopy = NewObject<UItemDataBase>(StaticClass());

	ItemCopy->ItemID = this->ItemID;
	ItemCopy->ItemCategory = this->ItemCategory;
	ItemCopy->AssetData = this->AssetData;
	ItemCopy->NumericData = this->NumericData;
	ItemCopy->TextData = this->TextData;

	return ItemCopy;
}

void UItemDataBase::SetQuantity(const int32 NewQuantity)
{
	if (Quantity != NewQuantity)
	{
		Quantity = FMath::Clamp(NewQuantity, 1, NumericData.MaxStackSize);
	}
}
