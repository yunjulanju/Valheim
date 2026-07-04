// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/HotBarSlot.h"
#include "ItemDragDropOperation.h"
#include "Inventory/InventoryComponent.h"

void UHotBarSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

void UHotBarSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (UItemDragDropOperation* DragOp = Cast<UItemDragDropOperation>(OutOperation))
	{
		DragOp->bFromHotbar = true;
		DragOp->SourceHotbarIndex = SlotIndex;
	}
}

bool UHotBarSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDropOperation* DragOp =
		Cast<UItemDragDropOperation>(InOperation);

	if (!DragOp || !DragOp->SourceInventory)
	{
		return false;
	}

	UInventoryComponent* Inventory = DragOp->SourceInventory;

	if (DragOp->bFromHotbar)
	{
		if (DragOp->SourceHotbarIndex == SlotIndex)
		{
			return true;
		}

		// 핫바는 실체가 아니라 인벤토리 인덱스를 참조만 하므로, 두 핫바 슬롯이 가리키는 인덱스를 서로 맞바꾸면 됨
		Inventory->SwapHotbarSlots(DragOp->SourceHotbarIndex, SlotIndex);
	}
	else
	{
		if (DragOp->SourceInventoryIndex == INDEX_NONE)
		{
			return false;
		}

		Inventory->MoveItemToHotbar(DragOp->SourceInventoryIndex, SlotIndex);
	}

	return true;
}