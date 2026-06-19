
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

    if (!DragOp || !DragOp->SourceItem)
    {
        return false;
    }

    UInventoryComponent* Inventory = DragOp->SourceInventory;

    if (!Inventory)
    {
        return false;
    }

	if (DragOp->bFromHotbar)
	{
		// ÇÖ¹Ù ½½·Ô³¢¸® ÀÚ¸® ±³È¯
		if (DragOp->SourceHotbarIndex == SlotIndex)
		{
			return true;
		}

		UItemDataBase* TargetSlotItem = Inventory->GetHotbarItem(SlotIndex);

		Inventory->SetHotbarItem(SlotIndex, DragOp->SourceItem);
		Inventory->SetHotbarItem(DragOp->SourceHotbarIndex, TargetSlotItem);
	}
	else
	{
		Inventory->MoveItemToHotbar(DragOp->SourceItem, SlotIndex);
	}

	return true;
}
