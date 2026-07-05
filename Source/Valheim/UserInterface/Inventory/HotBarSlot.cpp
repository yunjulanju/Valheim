// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/HotBarSlot.h"
#include "ItemDragDropOperation.h"
#include "Inventory/InventoryComponent.h"
#include "Character/Archer.h"

void UHotBarSlot::NativeConstruct()
{
	Super::NativeConstruct();
}

FReply UHotBarSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 핫바 슬롯은 이제 인벤토리 인덱스가 아니라 SlotIndex 자체로 아이템을 사용함
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (AArcher* Player = Cast<AArcher>(GetOwningPlayerPawn()))
		{
			Player->UseHotbarItem(SlotIndex);
		}
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
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

		// 두 핫바 슬롯이 들고 있는 아이템 실체를 서로 맞바꿈
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