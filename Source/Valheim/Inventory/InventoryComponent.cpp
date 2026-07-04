// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/InventoryComponent.h"
#include "Item/ItemSubsystem.h"
#include "Data/ItemPrimaryDataAsset.h"
#include "Engine/GameInstance.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryContents.OwningComponent = this;

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		InventoryContents.Items.SetNum(InventorySlotsCapacity);
		InventoryContents.MarkArrayDirty();

		HotbarToInventoryIndex.Init(INDEX_NONE, HotBarSlotsCapacity);
	}
}

UItemSubsystem* UInventoryComponent::GetItemSubsystem() const
{
	if (const UWorld* World = GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UItemSubsystem>();
		}
	}
	return nullptr;
}

// ===================== 조회 (Query) =====================

FInventoryItemInstance UInventoryComponent::GetInventoryItem(int32 InventoryIndex) const
{
	if (InventoryContents.Items.IsValidIndex(InventoryIndex))
	{
		return InventoryContents.Items[InventoryIndex];
	}
	return FInventoryItemInstance();
}

FInventoryItemInstance UInventoryComponent::GetHotbarItem(int32 HotbarIndex) const
{
	return GetInventoryItem(GetHotbarSlotInventoryIndex(HotbarIndex));
}

int32 UInventoryComponent::GetHotbarSlotInventoryIndex(int32 HotbarIndex) const
{
	if (HotbarToInventoryIndex.IsValidIndex(HotbarIndex))
	{
		return HotbarToInventoryIndex[HotbarIndex];
	}
	return INDEX_NONE;
}

TArray<FInventoryItemInstance> UInventoryComponent::GetInventoryContents() const
{
	return InventoryContents.Items;
}

int32 UInventoryComponent::GetUsedSlotCount() const
{
	int32 Count = 0;
	for (const FInventoryItemInstance& Item : InventoryContents.Items)
	{
		if (Item.IsValidItem())
		{
			Count++;
		}
	}
	return Count;
}

int32 UInventoryComponent::FindInventoryIndexByID(FName ItemID) const
{
	if (ItemID == NAME_None)
	{
		return INDEX_NONE;
	}

	for (int32 i = 0; i < InventoryContents.Items.Num(); i++)
	{
		if (InventoryContents.Items[i].ItemID == ItemID)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

int32 UInventoryComponent::FindNextPartialStackIndex(FName ItemID) const
{
	if (ItemID == NAME_None)
	{
		return INDEX_NONE;
	}

	UItemPrimaryDataAsset* ItemData = nullptr;
	if (const UItemSubsystem* Subsystem = GetItemSubsystem())
	{
		Subsystem->GetItemData(ItemID, ItemData);
	}
	if (!ItemData)
	{
		return INDEX_NONE;
	}

	for (int32 i = 0; i < InventoryContents.Items.Num(); i++)
	{
		const FInventoryItemInstance& Item = InventoryContents.Items[i];
		if (Item.ItemID == ItemID && Item.Quantity < ItemData->NumericData.MaxStackSize)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

// ===================== 추가 / 제거 (Mutation) =====================

int32 UInventoryComponent::CalculateAmountForFullStack(int32 CurrentQuantity, int32 MaxStackSize, int32 InitialRequestedAmount) const
{
	const int32 AmountToMakeFullStack = MaxStackSize - CurrentQuantity;
	return FMath::Min(InitialRequestedAmount, AmountToMakeFullStack);
}

void UInventoryComponent::AddNewItemAtIndex(int32 SlotIndex, FName ItemID, int32 AmountToAdd)
{
	if (!InventoryContents.Items.IsValidIndex(SlotIndex))
	{
		return;
	}

	FInventoryItemInstance& Slot = InventoryContents.Items[SlotIndex];
	Slot.ItemID = ItemID;
	Slot.Quantity = AmountToAdd;

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		InventoryContents.MarkItemDirty(Slot);
	}
}

FItemAddResult UInventoryComponent::HandleNoneStackableItems(FName ItemID, int32 RequestedAddAmount)
{
	const int32 EmptyIndex = InventoryContents.Items.IndexOfByPredicate([](const FInventoryItemInstance& Item)
		{
			return !Item.IsValidItem();
		});

	if (EmptyIndex == INDEX_NONE)
	{
		return FItemAddResult::AddedNone();
	}

	AddNewItemAtIndex(EmptyIndex, ItemID, RequestedAddAmount);
	OnInventoryUpdated.Broadcast();

	return FItemAddResult::AddedAll(RequestedAddAmount);
}

int32 UInventoryComponent::HandleStackableItems(FName ItemID, int32 RequestedAddAmount)
{
	UItemPrimaryDataAsset* ItemData = nullptr;
	if (const UItemSubsystem* Subsystem = GetItemSubsystem())
	{
		Subsystem->GetItemData(ItemID, ItemData);
	}
	if (!ItemData)
	{
		return 0;
	}

	int32 AmountToDistribute = RequestedAddAmount;

	int32 PartialIndex = FindNextPartialStackIndex(ItemID);
	while (PartialIndex != INDEX_NONE)
	{
		FInventoryItemInstance& Slot = InventoryContents.Items[PartialIndex];
		const int32 AmountToMakeFullStack = CalculateAmountForFullStack(Slot.Quantity, ItemData->NumericData.MaxStackSize, AmountToDistribute);

		Slot.Quantity += AmountToMakeFullStack;
		AmountToDistribute -= AmountToMakeFullStack;

		if (GetOwner() && GetOwner()->HasAuthority())
		{
			InventoryContents.MarkItemDirty(Slot);
		}

		if (AmountToDistribute <= 0)
		{
			OnInventoryUpdated.Broadcast();
			return RequestedAddAmount;
		}

		PartialIndex = FindNextPartialStackIndex(ItemID);
	}

	int32 EmptyIndex = InventoryContents.Items.IndexOfByPredicate([](const FInventoryItemInstance& Item)
		{
			return !Item.IsValidItem();
		});

	while (AmountToDistribute > 0 && EmptyIndex != INDEX_NONE)
	{
		const int32 AmountForNewStack = FMath::Min(AmountToDistribute, ItemData->NumericData.MaxStackSize);

		AddNewItemAtIndex(EmptyIndex, ItemID, AmountForNewStack);
		AmountToDistribute -= AmountForNewStack;

		EmptyIndex = InventoryContents.Items.IndexOfByPredicate([](const FInventoryItemInstance& Item)
			{
				return !Item.IsValidItem();
			});
	}

	OnInventoryUpdated.Broadcast();

	return RequestedAddAmount - AmountToDistribute;
}

FItemAddResult UInventoryComponent::HandleAddItem(FName ItemID, int32 RequestedAmount)
{
	if (!GetOwner() || ItemID == NAME_None || RequestedAmount <= 0)
	{
		return FItemAddResult::AddedNone();
	}

	UItemPrimaryDataAsset* ItemData = nullptr;
	if (const UItemSubsystem* Subsystem = GetItemSubsystem())
	{
		Subsystem->GetItemData(ItemID, ItemData);
	}
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleAddItem: ItemID %s not found in ItemSubsystem"), *ItemID.ToString());
		return FItemAddResult::AddedNone();
	}

	if (!ItemData->NumericData.bIsStackable)
	{
		return HandleNoneStackableItems(ItemID, RequestedAmount);
	}

	const int32 StackableAmountAdded = HandleStackableItems(ItemID, RequestedAmount);

	if (StackableAmountAdded == RequestedAmount)
	{
		return FItemAddResult::AddedAll(RequestedAmount);
	}
	if (StackableAmountAdded > 0)
	{
		return FItemAddResult::AddedPartial(StackableAmountAdded);
	}
	return FItemAddResult::AddedNone();
}

int32 UInventoryComponent::RemoveAmountOfItem(int32 InventoryIndex, int32 DesiredAmountToRemove)
{
	if (!InventoryContents.Items.IsValidIndex(InventoryIndex))
	{
		return 0;
	}

	FInventoryItemInstance& Slot = InventoryContents.Items[InventoryIndex];
	if (!Slot.IsValidItem())
	{
		return 0;
	}

	const int32 ActualAmountToRemove = FMath::Min(DesiredAmountToRemove, Slot.Quantity);
	Slot.Quantity -= ActualAmountToRemove;

	if (Slot.Quantity <= 0)
	{
		Slot.ItemID = NAME_None;
		Slot.Quantity = 0;
		RemoveItemFromHotbarIfPresent(InventoryIndex);
	}

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		InventoryContents.MarkItemDirty(Slot);
	}

	OnInventoryUpdated.Broadcast();
	return ActualAmountToRemove;
}

void UInventoryComponent::RemoveItemAtIndex(int32 InventoryIndex)
{
	if (!InventoryContents.Items.IsValidIndex(InventoryIndex))
	{
		return;
	}

	FInventoryItemInstance& Slot = InventoryContents.Items[InventoryIndex];
	Slot.ItemID = NAME_None;
	Slot.Quantity = 0;

	RemoveItemFromHotbarIfPresent(InventoryIndex);

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		InventoryContents.MarkItemDirty(Slot);
	}

	OnInventoryUpdated.Broadcast();
}

bool UInventoryComponent::SplitExistingStack(int32 InventoryIndex, int32 AmountToSplit)
{
	if (!InventoryContents.Items.IsValidIndex(InventoryIndex))
	{
		return false;
	}

	const FInventoryItemInstance& SourceSlot = InventoryContents.Items[InventoryIndex];
	if (!SourceSlot.IsValidItem() || AmountToSplit <= 0 || AmountToSplit >= SourceSlot.Quantity)
	{
		return false;
	}

	const int32 EmptyIndex = InventoryContents.Items.IndexOfByPredicate([](const FInventoryItemInstance& Item)
		{
			return !Item.IsValidItem();
		});
	if (EmptyIndex == INDEX_NONE)
	{
		return false;
	}

	const FName ItemID = SourceSlot.ItemID;
	const int32 ActualRemoved = RemoveAmountOfItem(InventoryIndex, AmountToSplit);
	AddNewItemAtIndex(EmptyIndex, ItemID, ActualRemoved);

	OnInventoryUpdated.Broadcast();
	return true;
}

// ===================== 이동 (Move) =====================

bool UInventoryComponent::MoveInventoryItem(int32 FromIndex, int32 ToIndex)
{
	if (!InventoryContents.Items.IsValidIndex(FromIndex) || !InventoryContents.Items.IsValidIndex(ToIndex))
	{
		return false;
	}

	if (FromIndex == ToIndex)
	{
		return true;
	}

	Swap(InventoryContents.Items[FromIndex], InventoryContents.Items[ToIndex]);

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		InventoryContents.MarkItemDirty(InventoryContents.Items[FromIndex]);
		InventoryContents.MarkItemDirty(InventoryContents.Items[ToIndex]);
	}

	for (int32& HotbarSlot : HotbarToInventoryIndex)
	{
		if (HotbarSlot == FromIndex)
		{
			HotbarSlot = ToIndex;
		}
		else if (HotbarSlot == ToIndex)
		{
			HotbarSlot = FromIndex;
		}
	}

	OnInventoryUpdated.Broadcast();
	return true;
}

bool UInventoryComponent::MoveItemToHotbar(int32 InventoryIndex, int32 HotbarIndex)
{
	if (!InventoryContents.Items.IsValidIndex(InventoryIndex) || !HotbarToInventoryIndex.IsValidIndex(HotbarIndex))
	{
		return false;
	}

	if (!InventoryContents.Items[InventoryIndex].IsValidItem())
	{
		return false;
	}

	HotbarToInventoryIndex[HotbarIndex] = InventoryIndex;

	OnInventoryUpdated.Broadcast();
	return true;
}

bool UInventoryComponent::MoveItemFromHotbarToInventorySlot(int32 HotbarIndex, int32 TargetInventoryIndex)
{
	const int32 SourceInventoryIndex = GetHotbarSlotInventoryIndex(HotbarIndex);
	if (SourceInventoryIndex == INDEX_NONE)
	{
		return false;
	}

	return MoveInventoryItem(SourceInventoryIndex, TargetInventoryIndex);
}

void UInventoryComponent::SwapHotbarSlots(int32 HotbarIndexA, int32 HotbarIndexB)
{
	if (!HotbarToInventoryIndex.IsValidIndex(HotbarIndexA) || !HotbarToInventoryIndex.IsValidIndex(HotbarIndexB))
	{
		return;
	}

	Swap(HotbarToInventoryIndex[HotbarIndexA], HotbarToInventoryIndex[HotbarIndexB]);
	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::SetHotbarSlot(int32 HotbarIndex, int32 InventoryIndex)
{
	if (HotbarToInventoryIndex.IsValidIndex(HotbarIndex))
	{
		HotbarToInventoryIndex[HotbarIndex] = InventoryIndex;
		OnInventoryUpdated.Broadcast();
	}
}

void UInventoryComponent::RemoveItemFromHotbarIfPresent(int32 InventoryIndex)
{
	for (int32& HotbarSlot : HotbarToInventoryIndex)
	{
		if (HotbarSlot == InventoryIndex)
		{
			HotbarSlot = INDEX_NONE;
		}
	}
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, InventoryContents);
	DOREPLIFETIME(UInventoryComponent, HotbarToInventoryIndex);
}