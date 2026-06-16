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
	UE_LOG(LogTemp, Warning, TEXT("RemoveSingleInstanceOfItem - OwningInventory: %s"),
		ItemToRemove->OwningInventory ? TEXT("Valid") : TEXT("Nullptr"));
	InventoryContents.RemoveSingle(ItemToRemove);
	OnInventoryUpdated.Broadcast();
}

int32 UInventoryComponent::RemoveAmountOfItem(UItemDataBase* ItemIn, int32 DesiredAmountToRemove)
{
	UE_LOG(LogTemp, Warning, TEXT("RemoveAmountOfItem - OwningInventory: %s"),
		ItemIn->OwningInventory ? TEXT("Valid") : TEXT("Nullptr"));

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
	int32 AmountToDistribute = RequestedAddAmount;

	// 기존 부분 스택에 채워넣기
	UItemDataBase* ExistingItem = FindNextPartialStack(InputItem);

	while (ExistingItem)
	{
		const int32 AmountToMakeFullStack = CalculateNumberForFullStack(ExistingItem, AmountToDistribute);

		ExistingItem->SetQuantity(ExistingItem->Quantity + AmountToMakeFullStack);
		AmountToDistribute -= AmountToMakeFullStack;

		if (AmountToDistribute <= 0)
		{
			OnInventoryUpdated.Broadcast();
			return RequestedAddAmount; // 전부 분배 완료
		}

		ExistingItem = FindNextPartialStack(InputItem);
	}

	// 새 슬롯에 추가 (슬롯 여유가 있을 때)
	while (AmountToDistribute > 0 && InventoryContents.Num() + 1 <= InventorySlotsCapacity)
	{
		const int32 AmountForNewStack = FMath::Min(AmountToDistribute, InputItem->NumericData.MaxStackSize);

		UItemDataBase* NewStackItem = InputItem->CreateItemCopy();
		NewStackItem->OwningInventory = this;
		NewStackItem->SetQuantity(AmountForNewStack);
		InventoryContents.Add(NewStackItem);

		AmountToDistribute -= AmountForNewStack;
	}

	OnInventoryUpdated.Broadcast();

	// 실제로 인벤토리에 들어간 양 = 요청한 양 - 못 들어간 양
	return RequestedAddAmount - AmountToDistribute;
}

FItemAddResult UInventoryComponent::HandleAddItem(UItemDataBase* InputItem)
{
	if (GetOwner())
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleAddItem - bIsStackable: %d"),
			InputItem->NumericData.bIsStackable);

		const int32 InitialRequestedAddAmount = InputItem->Quantity;

		if (!InputItem->NumericData.bIsStackable)
		{
			//UE_LOG(LogTemp, Warning, TEXT("HandleNoneStackableItems"))
			return HandleNoneStackableItems(InputItem, InitialRequestedAddAmount);
		}

		//내가 주운 아이템과 넣어야 할 값을 
		const int32 StackableAmountAdded = HandleStackableItems(InputItem, InitialRequestedAddAmount);

		if (StackableAmountAdded == InitialRequestedAddAmount)
		{
			//UE_LOG(LogTemp, Warning, TEXT("HandleAddItem AddedAll"))
			return FItemAddResult::AddedAll(InitialRequestedAddAmount);
		}
		if (StackableAmountAdded < InitialRequestedAddAmount && StackableAmountAdded >0)
		{
			//UE_LOG(LogTemp, Warning, TEXT("HandleAddItem AddedPartial"))
			return FItemAddResult::AddedPartial(InitialRequestedAddAmount);
		}
		if (StackableAmountAdded <= 0)
		{
			//UE_LOG(LogTemp, Warning, TEXT("HandleAddItem AddedNone"))
			return FItemAddResult::AddedNone();
		}
	}

	return FItemAddResult();
}

void UInventoryComponent::AddNewItem(UItemDataBase* Item, int32 AmountToAdd)
{
	//UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::AddNewItem"))
	UItemDataBase* NewItem;

	UE_LOG(LogTemp, Warning, TEXT("AddNewItem - bIsCopy: %d, bIsPickup: %d"),
		Item->bIsCopy, Item->bIsPickup);

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

	UE_LOG(LogTemp, Warning, TEXT("AddNewItem - OwningInventory after set: %s"),
		NewItem->OwningInventory ? TEXT("Valid") : TEXT("Nullptr"));

	InventoryContents.Add(NewItem);
	OnInventoryUpdated.Broadcast();
}