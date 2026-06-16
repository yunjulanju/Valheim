// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemDataBase.h"
#include "Inventory/InventoryComponent.h"

UItemDataBase::UItemDataBase() : bIsCopy(false), bIsPickup(false)
{
}

void UItemDataBase::ResetItemFlags()
{
	bIsCopy = false;
	bIsPickup = false;
}

UItemDataBase* UItemDataBase::CreateItemCopy() const
{
	UItemDataBase* ItemCopy = NewObject<UItemDataBase>(StaticClass());

	ItemCopy->ItemID = this->ItemID;
	ItemCopy->ItemCategory = this->ItemCategory;
	ItemCopy->AssetData = this->AssetData;
	ItemCopy->NumericData = this->NumericData;
	ItemCopy->TextData = this->TextData;
	ItemCopy->bIsCopy = true;

	return ItemCopy;
}

void UItemDataBase::SetQuantity(const int32 NewQuantity)
{
	//UE_LOG(LogTemp, Warning, TEXT("UItemDataBase SetQuantity - NewQuantity %d"), NewQuantity);
	if (Quantity != NewQuantity)
	{
		Quantity = FMath::Clamp(NewQuantity, 0, NumericData.MaxStackSize);
		UE_LOG(LogTemp, Warning, TEXT("UItemDataBase SetQuantity - Clamp Quantity %d"), Quantity);
		if(OwningInventory)
		{
			if (Quantity <= 0)
			{
				OwningInventory->RemoveSingleInstanceOfItem(this);
			}
		}
	}
}


