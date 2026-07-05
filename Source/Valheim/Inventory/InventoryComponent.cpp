// Fill out your copyright notice in the Description page of Project Settings.

#include "Inventory/InventoryComponent.h"
#include "Item/ItemSubsystem.h"
#include "Data/ItemPrimaryDataAsset.h"
#include "Engine/GameInstance.h"
#include "Net/UnrealNetwork.h"
#include <Character/Archer.h>

UInventoryComponent::UInventoryComponent()
{
	SetIsReplicatedByDefault(true);
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	InventoryContents.OwningComponent = this;
	HotbarContents.OwningComponent = this;

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		InventoryContents.Items.SetNum(InventorySlotsCapacity);
		InventoryContents.MarkArrayDirty();

		HotbarContents.Items.SetNum(HotBarSlotsCapacity);
		HotbarContents.MarkArrayDirty();
		OnInventoryUpdated.Broadcast();
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

// ===================== Query =====================

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
	if (HotbarContents.Items.IsValidIndex(HotbarIndex))
	{
		return HotbarContents.Items[HotbarIndex];
	}
	return FInventoryItemInstance();
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

// ===================== Add / Remove (Mutation) =====================

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

	InventoryContents.MarkItemDirty(Slot);
}

void UInventoryComponent::SwapItemContents(FInventoryItemInstance& A, FInventoryItemInstance& B)
{
	FName TempID = A.ItemID;
	int32 TempQty = A.Quantity;

	A.ItemID = B.ItemID;
	A.Quantity = B.Quantity;

	B.ItemID = TempID;
	B.Quantity = TempQty;
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

		InventoryContents.MarkItemDirty(Slot);

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

// ---- HandleAddItem ----
FItemAddResult UInventoryComponent::HandleAddItem(FName ItemID, int32 RequestedAmount)
{
	if (!GetOwner() || ItemID == NAME_None || RequestedAmount <= 0)
	{
		return FItemAddResult::AddedNone();
	}

	if (GetOwner()->HasAuthority())
	{
		return HandleAddItem_Internal(ItemID, RequestedAmount);
	}

	Server_HandleAddItem(ItemID, RequestedAmount);
	return FItemAddResult::AddedNone();
}

FItemAddResult UInventoryComponent::HandleAddItem_Internal(FName ItemID, int32 RequestedAmount)
{
	UItemPrimaryDataAsset* ItemData = nullptr;
	if (const UItemSubsystem* Subsystem = GetItemSubsystem())
	{
		Subsystem->GetItemData(ItemID, ItemData);
	}
	if (!ItemData)
	{
		UE_LOG(LogTemp, Warning, TEXT("HandleAddItem_Internal: ItemID %s not found in ItemSubsystem"), *ItemID.ToString());
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

void UInventoryComponent::Server_HandleAddItem_Implementation(FName ItemID, int32 RequestedAmount)
{
	if (!GetOwner() || ItemID == NAME_None || RequestedAmount <= 0)
	{
		return;
	}

	HandleAddItem_Internal(ItemID, RequestedAmount);
}

// ---- RemoveAmountOfItem ----
int32 UInventoryComponent::RemoveAmountOfItem(int32 InventoryIndex, int32 DesiredAmountToRemove)
{
	if (!GetOwner())
	{
		return 0;
	}

	if (GetOwner()->HasAuthority())
	{
		return RemoveAmountOfItem_Internal(InventoryIndex, DesiredAmountToRemove);
	}

	ServerRemoveAmountOfItem(InventoryIndex, DesiredAmountToRemove);
	return 0;
}

int32 UInventoryComponent::RemoveAmountOfItem_Internal(int32 InventoryIndex, int32 DesiredAmountToRemove)
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
	}

	InventoryContents.MarkItemDirty(Slot);

	OnInventoryUpdated.Broadcast();
	return ActualAmountToRemove;
}

void UInventoryComponent::ServerRemoveAmountOfItem_Implementation(int32 InventoryIndex, int32 DesiredAmountToRemove)
{
	RemoveAmountOfItem_Internal(InventoryIndex, DesiredAmountToRemove);
}

// ---- RemoveAmountOfHotbarItem ----
int32 UInventoryComponent::RemoveAmountOfHotbarItem(int32 HotbarIndex, int32 DesiredAmountToRemove)
{
	if (!GetOwner())
	{
		return 0;
	}

	if (GetOwner()->HasAuthority())
	{
		return RemoveAmountOfHotbarItem_Internal(HotbarIndex, DesiredAmountToRemove);
	}

	ServerRemoveAmountOfHotbarItem(HotbarIndex, DesiredAmountToRemove);
	return 0;
}

int32 UInventoryComponent::RemoveAmountOfHotbarItem_Internal(int32 HotbarIndex, int32 DesiredAmountToRemove)
{
	if (!HotbarContents.Items.IsValidIndex(HotbarIndex))
	{
		return 0;
	}

	FInventoryItemInstance& Slot = HotbarContents.Items[HotbarIndex];
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
	}

	HotbarContents.MarkItemDirty(Slot);

	OnInventoryUpdated.Broadcast();
	return ActualAmountToRemove;
}

void UInventoryComponent::ServerRemoveAmountOfHotbarItem_Implementation(int32 HotbarIndex, int32 DesiredAmountToRemove)
{
	RemoveAmountOfHotbarItem_Internal(HotbarIndex, DesiredAmountToRemove);
}

// ---- RemoveItemAtIndex ----
void UInventoryComponent::RemoveItemAtIndex(int32 InventoryIndex)
{
	if (!GetOwner())
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		RemoveItemAtIndex_Internal(InventoryIndex);
		return;
	}

	ServerRemoveItemAtIndex(InventoryIndex);
}

void UInventoryComponent::RemoveItemAtIndex_Internal(int32 InventoryIndex)
{
	if (!InventoryContents.Items.IsValidIndex(InventoryIndex))
	{
		return;
	}

	FInventoryItemInstance& Slot = InventoryContents.Items[InventoryIndex];
	Slot.ItemID = NAME_None;
	Slot.Quantity = 0;

	InventoryContents.MarkItemDirty(Slot);

	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::ServerRemoveItemAtIndex_Implementation(int32 InventoryIndex)
{
	RemoveItemAtIndex_Internal(InventoryIndex);
}

// ---- SplitExistingStack ----
bool UInventoryComponent::SplitExistingStack(int32 InventoryIndex, int32 AmountToSplit)
{
	if (!GetOwner())
	{
		return false;
	}

	if (!InventoryContents.Items.IsValidIndex(InventoryIndex))
	{
		return false;
	}

	const FInventoryItemInstance& SourceSlot = InventoryContents.Items[InventoryIndex];
	if (!SourceSlot.IsValidItem() || AmountToSplit <= 0 || AmountToSplit >= SourceSlot.Quantity)
	{
		return false;
	}

	if (GetOwner()->HasAuthority())
	{
		return SplitExistingStack_Internal(InventoryIndex, AmountToSplit);
	}

	ServerSplitExistingStack(InventoryIndex, AmountToSplit);
	return true;
}

bool UInventoryComponent::SplitExistingStack_Internal(int32 InventoryIndex, int32 AmountToSplit)
{
	if (!InventoryContents.Items.IsValidIndex(InventoryIndex))
	{
		return false;
	}

	FInventoryItemInstance& SourceSlot = InventoryContents.Items[InventoryIndex];
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

	const int32 ActualRemoved = FMath::Min(AmountToSplit, SourceSlot.Quantity);
	SourceSlot.Quantity -= ActualRemoved;
	if (SourceSlot.Quantity <= 0)
	{
		SourceSlot.ItemID = NAME_None;
		SourceSlot.Quantity = 0;
	}
	InventoryContents.MarkItemDirty(SourceSlot);

	AddNewItemAtIndex(EmptyIndex, ItemID, ActualRemoved);

	OnInventoryUpdated.Broadcast();
	return true;
}

void UInventoryComponent::ServerSplitExistingStack_Implementation(int32 InventoryIndex, int32 AmountToSplit)
{
	SplitExistingStack_Internal(InventoryIndex, AmountToSplit);
}

// ===================== Move =====================

// ---- MoveInventoryItem (Inventory ↔ Inventory) ----
bool UInventoryComponent::MoveInventoryItem(int32 FromIndex, int32 ToIndex)
{
	if (!GetOwner())
	{
		return false;
	}

	if (!InventoryContents.Items.IsValidIndex(FromIndex) || !InventoryContents.Items.IsValidIndex(ToIndex))
	{
		return false;
	}

	if (FromIndex == ToIndex)
	{
		return true;
	}

	if (GetOwner()->HasAuthority())
	{
		return MoveInventoryItem_Internal(FromIndex, ToIndex);
	}

	ServerMoveInventoryItem(FromIndex, ToIndex);
	return true;
}

bool UInventoryComponent::MoveInventoryItem_Internal(int32 FromIndex, int32 ToIndex)
{
	if (!InventoryContents.Items.IsValidIndex(FromIndex) || !InventoryContents.Items.IsValidIndex(ToIndex))
	{
		return false;
	}

	if (FromIndex == ToIndex)
	{
		return true;
	}

	SwapItemContents(InventoryContents.Items[FromIndex], InventoryContents.Items[ToIndex]);

	InventoryContents.MarkItemDirty(InventoryContents.Items[FromIndex]);
	InventoryContents.MarkItemDirty(InventoryContents.Items[ToIndex]);

	OnInventoryUpdated.Broadcast();
	return true;
}

void UInventoryComponent::ServerMoveInventoryItem_Implementation(int32 FromIndex, int32 ToIndex)
{
	MoveInventoryItem_Internal(FromIndex, ToIndex);
}

// ---- MoveItemToHotbar (Inventory ↔ Hotbar) ----
bool UInventoryComponent::MoveItemToHotbar(int32 InventoryIndex, int32 HotbarIndex)
{
	if (!GetOwner())
	{
		return false;
	}

	if (!InventoryContents.Items.IsValidIndex(InventoryIndex) || !HotbarContents.Items.IsValidIndex(HotbarIndex))
	{
		return false;
	}

	const FInventoryItemInstance& InvSlot = InventoryContents.Items[InventoryIndex];
	if (!InvSlot.IsValidItem())
	{
		return false;
	}

	if (GetOwner()->HasAuthority())
	{
		return MoveItemToHotbar_Internal(InventoryIndex, HotbarIndex);
	}

	ServerMoveItemToHotbar(InventoryIndex, HotbarIndex);
	return true;
}

bool UInventoryComponent::MoveItemToHotbar_Internal(int32 InventoryIndex, int32 HotbarIndex)
{
	if (!InventoryContents.Items.IsValidIndex(InventoryIndex) || !HotbarContents.Items.IsValidIndex(HotbarIndex))
	{
		return false;
	}

	FInventoryItemInstance& InvSlot = InventoryContents.Items[InventoryIndex];
	FInventoryItemInstance& HotbarSlot = HotbarContents.Items[HotbarIndex];

	SwapItemContents(InvSlot, HotbarSlot);

	InventoryContents.MarkItemDirty(InvSlot);
	HotbarContents.MarkItemDirty(HotbarSlot);

	if (AArcher* ArcherCharacter = Cast<AArcher>(GetOwner()))
	{
		ArcherCharacter->RefreshActiveHotbarEquip();
	}

	OnInventoryUpdated.Broadcast();
	return true;
}

void UInventoryComponent::ServerMoveItemToHotbar_Implementation(int32 InventoryIndex, int32 HotbarIndex)
{
	MoveItemToHotbar_Internal(InventoryIndex, HotbarIndex);
}

// ---- MoveItemFromHotbarToInventorySlot (Hotbar ↔ Inventory) ----
bool UInventoryComponent::MoveItemFromHotbarToInventorySlot(int32 HotbarIndex, int32 TargetInventoryIndex)
{
	if (!GetOwner())
	{
		return false;
	}

	if (!HotbarContents.Items.IsValidIndex(HotbarIndex) || !InventoryContents.Items.IsValidIndex(TargetInventoryIndex))
	{
		return false;
	}

	const FInventoryItemInstance& HotbarSlot = HotbarContents.Items[HotbarIndex];
	if (!HotbarSlot.IsValidItem())
	{
		return false;
	}

	if (GetOwner()->HasAuthority())
	{
		return MoveItemFromHotbarToInventorySlot_Internal(HotbarIndex, TargetInventoryIndex);
	}

	ServerMoveItemFromHotbarToInventorySlot(HotbarIndex, TargetInventoryIndex);
	return true;
}

bool UInventoryComponent::MoveItemFromHotbarToInventorySlot_Internal(int32 HotbarIndex, int32 TargetInventoryIndex)
{
	if (!HotbarContents.Items.IsValidIndex(HotbarIndex) || !InventoryContents.Items.IsValidIndex(TargetInventoryIndex))
	{
		return false;
	}

	FInventoryItemInstance& HotbarSlot = HotbarContents.Items[HotbarIndex];
	FInventoryItemInstance& InvSlot = InventoryContents.Items[TargetInventoryIndex];

	SwapItemContents(HotbarSlot, InvSlot);

	HotbarContents.MarkItemDirty(HotbarSlot);
	InventoryContents.MarkItemDirty(InvSlot);

	if (AArcher* ArcherCharacter = Cast<AArcher>(GetOwner()))
	{
		ArcherCharacter->RefreshActiveHotbarEquip();
	}

	OnInventoryUpdated.Broadcast();
	return true;
}

void UInventoryComponent::ServerMoveItemFromHotbarToInventorySlot_Implementation(int32 HotbarIndex, int32 TargetInventoryIndex)
{
	MoveItemFromHotbarToInventorySlot_Internal(HotbarIndex, TargetInventoryIndex);
}

// ---- SwapHotbarSlots (Hotbar ↔ Hotbar) ----
void UInventoryComponent::SwapHotbarSlots(int32 HotbarIndexA, int32 HotbarIndexB)
{
	if (!GetOwner())
	{
		return;
	}

	if (!HotbarContents.Items.IsValidIndex(HotbarIndexA) || !HotbarContents.Items.IsValidIndex(HotbarIndexB))
	{
		return;
	}

	if (GetOwner()->HasAuthority())
	{
		SwapHotbarSlots_Internal(HotbarIndexA, HotbarIndexB);
		return;
	}

	ServerSwapHotbarSlots(HotbarIndexA, HotbarIndexB);
}

void UInventoryComponent::SwapHotbarSlots_Internal(int32 HotbarIndexA, int32 HotbarIndexB)
{
	if (!HotbarContents.Items.IsValidIndex(HotbarIndexA) || !HotbarContents.Items.IsValidIndex(HotbarIndexB))
	{
		return;
	}

	SwapItemContents(HotbarContents.Items[HotbarIndexA], HotbarContents.Items[HotbarIndexB]);

	HotbarContents.MarkItemDirty(HotbarContents.Items[HotbarIndexA]);
	HotbarContents.MarkItemDirty(HotbarContents.Items[HotbarIndexB]);

	if (AArcher* ArcherCharacter = Cast<AArcher>(GetOwner()))
	{
		ArcherCharacter->RefreshActiveHotbarEquip();
	}

	OnInventoryUpdated.Broadcast();
}

void UInventoryComponent::ServerSwapHotbarSlots_Implementation(int32 HotbarIndexA, int32 HotbarIndexB)
{
	SwapHotbarSlots_Internal(HotbarIndexA, HotbarIndexB);
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, InventoryContents);
	DOREPLIFETIME(UInventoryComponent, HotbarContents);
}