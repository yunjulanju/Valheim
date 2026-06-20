// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemDataBase.h"
#include "Inventory/InventoryComponent.h"
#include "Character/Archer.h"

UItemDataBase::UItemDataBase() : bIsCopy(false), bIsPickup(false)
{
}

void UItemDataBase::ResetItemFlags()
{
	bIsCopy = false;
	bIsPickup = false;
}

void UItemDataBase::Use(AArcher* User)
{
	if (ItemCategory.ItemCategory != EItemCategory::Consumable || !User)
	{
		UE_LOG(LogTemp, Warning, TEXT("UConsumableItemDataBase Null"));
		return;
	}
	if (User->GetCurrentHP() <= 0)
	{
		return;
	}

	switch (ItemCategory.ItemType)
	{
	case EItemType::Heal:
		User->AddHP(ItemCategory.Value);
		break;
	case EItemType::Damage:
		//우선 데미지 입는 거로 햇는데, 공격량 증가하는 것도 괜찮을 수도
		User->AddHP(-1 * ItemCategory.Value);
		break;
	case EItemType::Defense:
		//데미지 방어 등
		break;
	}
	OwningInventory->RemoveAmountOfItem(this, 1);
}

UItemDataBase* UItemDataBase::CreateItemCopy() const
{
	UItemDataBase* ItemCopy = NewObject<UItemDataBase>();

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

	if (Quantity != NewQuantity)
	{
		Quantity = FMath::Clamp(NewQuantity, 0, NumericData.MaxStackSize);

		if (OwningInventory)
		{
			if (Quantity <= 0)
			{
				OwningInventory->RemoveSingleInstanceOfItem(this);
			}
		}
	}
}


