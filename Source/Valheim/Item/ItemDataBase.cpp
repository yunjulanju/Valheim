// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemDataBase.h"
#include "Inventory/InventoryComponent.h"
#include "Character/Archer.h"
#include <Character/ArcherPS.h>

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
	AArcherPS* ArcherPS = User->GetPlayerState<AArcherPS>();
	if (ArcherPS)
	{
		ArcherPS->UpdateQuestProgressByEvent(EQuestType::UseItem, ItemID, 1);
	}

	switch (ItemCategory.ItemType)
	{
	case EItemType::Heal:
		User->AddHP(ItemCategory.Value);
		break;
	case EItemType::Damage:
		User->AddHP(-1 * ItemCategory.Value);
		break;
	case EItemType::Defense:
		break;
	}

	// TODO(3 step, UI refactor): UInventoryComponent no longer accepts a raw UObject item pointer
	// (RemoveAmountOfItem is now int32 InventoryIndex based). Once callers (InventoryItemSlot etc.)
	// move to FInventoryItemInstance/index, this should call
	// UInventoryComponent::RemoveAmountOfItem(int32 InventoryIndex, N) directly instead.
}

UItemDataBase* UItemDataBase::CreateItemCopy(UObject* Outer) const
{
	UItemDataBase* ItemCopy = NewObject<UItemDataBase>(Outer);

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

		// TODO(3 step, UI refactor): RemoveSingleInstanceOfItem no longer exists on UInventoryComponent.
		// This UObject-based item no longer knows its own inventory index, so it cannot clear itself.
		// Once UI moves off UItemDataBase, quantity-hits-zero cleanup should happen in
		// UInventoryComponent::RemoveAmountOfItem itself (it already does this).
	}
}