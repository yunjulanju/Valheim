// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryComponent.h"
#include "Item/ItemDataBase.h"
#include "Net/UnrealNetwork.h" 

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);
	//bReplicateUsingRegisteredSubObjectList = true;
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	HotBarContents.SetNum(HotBarSlotsCapacity);
	InventoryContents.SetNum(InventorySlotsCapacity);
}

UItemDataBase* UInventoryComponent::FindMatchingItem(UItemDataBase* ItemIn) const
{
	if (ItemIn)
	{
		if (InventoryContents.Contains(ItemIn))
		{
			return ItemIn;
		}
		if (HotBarContents.Contains(ItemIn))
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
	if (!ItemIn)
	{
		UE_LOG(LogTemp, Error, TEXT("FindNextPartialStack: ItemIn is NULL"));
		return nullptr;
	}

	const TArray<TObjectPtr<UItemDataBase>>::ElementType* Result = nullptr;

	for (const TObjectPtr<UItemDataBase>& InventoryItem : InventoryContents)
	{
		if (InventoryItem && InventoryItem->ItemID == ItemIn->ItemID && !InventoryItem->IsFullStack())
		{
			Result = &InventoryItem;
			break;
		}
	}

	if (Result)
	{
		return *Result;
	}
	return nullptr;
	
}

void UInventoryComponent::RemoveSingleInstanceOfItem(UItemDataBase* ItemToRemove)
{
	int32 FoundIndex = InventoryContents.Find(ItemToRemove);
	if (FoundIndex != INDEX_NONE)
	{
		InventoryContents[FoundIndex] = nullptr;

		if (GetOwner() && GetOwner()->HasAuthority())
		{
			RemoveReplicatedSubObject(ItemToRemove);
		}
	}
	RemoveItemFromHotbarIfPresent(ItemToRemove);
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

void UInventoryComponent::SetHotbarItem(int32 index, UItemDataBase* Item)
{
	if (HotBarContents.IsValidIndex(index))
	{
		HotBarContents[index] = Item;
		OnInventoryUpdated.Broadcast();
	}
}

bool UInventoryComponent::MoveInventoryItem(int32 FromIndex, int32 ToIndex)
{
	if (!InventoryContents.IsValidIndex(FromIndex) || !InventoryContents.IsValidIndex(ToIndex))
	{
		return false;
	}

	if (FromIndex == ToIndex)
	{
		return true;
	}

	UItemDataBase* Item = InventoryContents[ToIndex];
	InventoryContents[ToIndex] = InventoryContents[FromIndex];
	InventoryContents[FromIndex] = Item;

	OnInventoryUpdated.Broadcast();
	return true;
}

int32 UInventoryComponent::CalculateNumberForFullStack(UItemDataBase* StackableItem, int32 InitialRequestedAddAmount)
{
	const int32 AddAmountToMakeFullStack = StackableItem->NumericData.MaxStackSize - StackableItem->Quantity;
	return FMath::Min(InitialRequestedAddAmount, AddAmountToMakeFullStack);
}

FItemAddResult UInventoryComponent::HandleNoneStackableItems(UItemDataBase* InputItem, int32 RequestedAddAmount)
{
	int32 NullCount = 0;
	for (const auto& Item : InventoryContents)
	{
		if (Item == nullptr) NullCount++;
	}

	if (!InventoryContents.Contains(nullptr))
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
	int32 EmptyIndex = InventoryContents.Find(nullptr);
	while (AmountToDistribute > 0 && EmptyIndex != INDEX_NONE)
	{
		const int32 AmountForNewStack = FMath::Min(AmountToDistribute, InputItem->NumericData.MaxStackSize);

		UItemDataBase* NewStackItem = InputItem->CreateItemCopy(this);
		NewStackItem->OwningInventory = this;
		NewStackItem->SetQuantity(AmountForNewStack);

		InventoryContents[EmptyIndex] = NewStackItem; 
		
		if (GetOwner() && GetOwner()->HasAuthority())
		{
			AddReplicatedSubObject(NewStackItem);
			UE_LOG(LogTemp, Warning, TEXT("AddReplicatedSubObject called for %s"), *NewStackItem->GetName());
		}

		AmountToDistribute -= AmountForNewStack;
		EmptyIndex = InventoryContents.Find(nullptr);   // 다음 빈 슬롯 다시 탐색
	}

	OnInventoryUpdated.Broadcast();

	// 실제로 인벤토리에 들어간 양 = 요청한 양 - 못 들어간 양
	return RequestedAddAmount - AmountToDistribute;
}

int32 UInventoryComponent::GetUsedSlotCount() const
{
	int32 Count = 0;
	for (const TObjectPtr<UItemDataBase>& Item : InventoryContents)
	{
		if (Item != nullptr)
		{
			Count++;
		}
	}
	return Count;
}

FItemAddResult UInventoryComponent::HandleAddItem(UItemDataBase* InputItem)
{
	if (GetOwner())
	{
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
	UItemDataBase* NewItem;

	if (Item->bIsCopy || Item->bIsPickup)
	{ //땅에 있는 것을 줍는 거라 copy가 필요 없음.
		NewItem = Item;
		NewItem->ResetItemFlags();
		NewItem->Rename(nullptr, this);
	}
	else
	{
		NewItem = Item->CreateItemCopy(this);
	}

	NewItem->OwningInventory = this;
	NewItem->SetQuantity(AmountToAdd);

	int32 EmptyIndex = InventoryContents.Find(nullptr);
	if (EmptyIndex != INDEX_NONE)
	{
		InventoryContents[EmptyIndex] = NewItem;

		if (GetOwner() && GetOwner()->HasAuthority())
		{
			AddReplicatedSubObject(NewItem);
		}
	}
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::RemoveItemFromInventoryOnly(UItemDataBase* ItemToRemove)
{
	int32 FoundIndex = InventoryContents.Find(ItemToRemove);
	if (FoundIndex != INDEX_NONE)
	{
		InventoryContents[FoundIndex] = nullptr;

		if (GetOwner() && GetOwner()->HasAuthority())
		{
			RemoveReplicatedSubObject(ItemToRemove);
		}
	}
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::RemoveItemFromHotbarIfPresent(UItemDataBase* ItemToCheck)
{
	for (int32 i = 0; i < HotBarContents.Num(); i++)
	{
		if (HotBarContents[i] == ItemToCheck)
		{
			HotBarContents[i] = nullptr;
		}
	}
}

bool UInventoryComponent::MoveItemToHotbar(UItemDataBase* ItemIn, int32 HotbarIndex)
{
	if (!ItemIn || !HotBarContents.IsValidIndex(HotbarIndex))
	{
		return false;
	}

	int32 FoundIndex = InventoryContents.Find(ItemIn);
	if (FoundIndex == INDEX_NONE)
	{
		return false;
	}

	RemoveItemFromHotbarIfPresent(ItemIn);

	HotBarContents[HotbarIndex] = ItemIn;
	InventoryContents[FoundIndex] = nullptr;

	OnInventoryUpdated.Broadcast();
	return true;
}

bool UInventoryComponent::MoveItemFromHotbarToInventorySlot(int32 HotbarIndex, int32 TargetInventoryIndex)
{
	if (!HotBarContents.IsValidIndex(HotbarIndex) || !InventoryContents.IsValidIndex(TargetInventoryIndex))
	{
		return false;
	}

	UItemDataBase* HotbarItem = HotBarContents[HotbarIndex];
	if (!HotbarItem)
	{
		return false;
	}

	// 목표 슬롯에 이미 아이템이 있으면 자리 교환, 비어있으면 그냥 채움
	UItemDataBase* ExistingTarget = InventoryContents[TargetInventoryIndex];

	HotBarContents[HotbarIndex] = nullptr;
	InventoryContents[TargetInventoryIndex] = HotbarItem;

	if (ExistingTarget)
	{
		HotBarContents[HotbarIndex] = ExistingTarget; // 원래 핫바 자리에 교환 대상 넣기
	}

	OnInventoryUpdated.Broadcast();
	return true;
}

void UInventoryComponent::OnRep_Items()
{
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, InventoryContents);
}
