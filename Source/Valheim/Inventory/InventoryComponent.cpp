// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryComponent.h"
#include "Item/ItemDataBase.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();


}

UItemDataBase* UInventoryComponent::FindMatchingItem(UItemDataBase* ItemIn) const
{
	if (ItemIn)
	{
		if (InventoryContents.Contains(ItemIn))
		{
			return ItemIn;
		}
	}
	return nullptr;
}

UItemDataBase* UInventoryComponent::FindNextItemByID(UItemDataBase* ItemIn) const
{
	if (ItemIn)
	{
		if (const TArray<TObjectPtr<UItemDataBase>>::ElementType* Result = InventoryContents.FindByKey(ItemIn))
		{
			return *Result;
		}
	}
	return nullptr;
}

UItemDataBase* UInventoryComponent::FindNextPartialStack(UItemDataBase* ItemIn) const
{
	const TArray<TObjectPtr<UItemDataBase>>::ElementType* Result = nullptr;

	for (const TObjectPtr<UItemDataBase>& InventoryItem : InventoryContents)
	{
		if (InventoryItem->ItemID == ItemIn->ItemID && !InventoryItem->IsFullStack())
		{
			Result = &InventoryItem;
			break;
		}
	}

	if (Result)
	{
		return *Result;
	}
	else
	{
		return nullptr;
	}
	
}

void UInventoryComponent::RemoveSingleInstanceOfItem(UItemDataBase* ItemIn)
{
}

int32 UInventoryComponent::RemoveAmountOfItem(UItemDataBase* ItemIn, int32 DesiredAmountToRemove)
{
	return int32();
}

void UInventoryComponent::SplitExistingStack(UItemDataBase* ItemIn, const int32 AmountToSplit)
{
}

int32 UInventoryComponent::HandleStackableItems(UItemDataBase*, int32 RequestedAddAmount)
{
	return int32();
}

int32 UInventoryComponent::CalculateNumberForFullStack(UItemDataBase* ExistingItem, int32 InitialRequestedAddAmount)
{
	return int32();
}

void UInventoryComponent::AddNewItem(UItemDataBase* Item, int32 AmountToAdd)
{
}

