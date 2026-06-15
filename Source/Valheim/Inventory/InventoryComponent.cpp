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

void UInventoryComponent::RemoveSingleInstanceOfItem(UItemDataBase* ItemToRemove)
{
	InventoryContents.RemoveSingle(ItemToRemove);
	OnInventoryUpdated.Broadcast();
}

int32 UInventoryComponent::RemoveAmountOfItem(UItemDataBase* ItemIn, int32 DesiredAmountToRemove)
{
	const int32 ActualAmountToRemove = FMath::Min(DesiredAmountToRemove, ItemIn->Quantity);
	ItemIn->SetQuantity(ItemIn->Quantity - ActualAmountToRemove);
	OnInventoryUpdated.Broadcast();

	return ActualAmountToRemove;
}

void UInventoryComponent::SplitExistingStack(UItemDataBase* ItemIn, const int32 AmountToSplit)
{
	if (!(InventoryContents.Num() + 1 > InventorySlotsCapacity))
	{
		RemoveAmountOfItem(ItemIn, AmountToSplit);
		AddNewItem(ItemIn, AmountToSplit);
	}
}

int32 UInventoryComponent::CalculateNumberForFullStack(UItemDataBase* StackableItem, int32 InitialRequestedAddAmount)
{
	const int32 AddAmountToMakeFullStack = StackableItem->NumericData.MaxStackSize - StackableItem->Quantity;
	return FMath::Min(InitialRequestedAddAmount, AddAmountToMakeFullStack);
}

FItemAddResult UInventoryComponent::HandleNoneStackableItems(UItemDataBase* InputItem, int32 RequestedAddAmount)
{
	if (InventoryContents.Num() + 1 > InventorySlotsCapacity)
	{
		return FItemAddResult::AddedNone();
	}

	AddNewItem(InputItem, RequestedAddAmount);

	return FItemAddResult::AddedAll(RequestedAddAmount);
}

int32 UInventoryComponent::HandleStackableItems(UItemDataBase* InputItem, int32 RequestedAddAmount)
{

	
	return 0;
}

FItemAddResult UInventoryComponent::HandleAddItem(UItemDataBase* InputItem)
{
	if (GetOwner())
	{
		const int32 InitialRequestedAddAmount = InputItem->Quantity;

		if (!InputItem->NumericData.bIsStackable)
		{
			return HandleNoneStackableItems(InputItem, InitialRequestedAddAmount);
		}

		const int32 StackableAmountAdded = HandleStackableItems(InputItem, InitialRequestedAddAmount);

		if (StackableAmountAdded == InitialRequestedAddAmount)
		{
			return FItemAddResult::AddedAll(InitialRequestedAddAmount);
		}
		if (StackableAmountAdded < InitialRequestedAddAmount && StackableAmountAdded >0)
		{
			return FItemAddResult::AddedPartial(InitialRequestedAddAmount);
		}
		if (StackableAmountAdded <= 0)
		{
			return FItemAddResult::AddedNone();
		}
	}

	return FItemAddResult();
}

void UInventoryComponent::AddNewItem(UItemDataBase* Item, int32 AmountToAdd)
{
	UItemDataBase* NewItem;

	if (Item->bIsCopy || Item->bIsPickup)
	{ //땅에 있는 것을 줍는 거라 copy가 필요 없음.
		NewItem = Item;
		NewItem->ResetItemFlags();
	}
	else
	{
		NewItem = Item->CreateItemCopy();
	}

	NewItem->OwningInventory = this;
	NewItem->SetQuantity(AmountToAdd);

	InventoryContents.Add(NewItem);
	OnInventoryUpdated.Broadcast();
}