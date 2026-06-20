// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/InventoryItemSlot.h"
#include "UserInterface/Inventory/InventoryToolTip.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Item/ItemDataBase.h"
#include "UserInterface/Inventory/DragItemVisual.h"
#include "ItemDragDropOperation.h"
#include "Character/Archer.h"
#include "Inventory/InventoryComponent.h"


void UInventoryItemSlot::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UInventoryItemSlot::NativeConstruct()
{
	Super::NativeConstruct();

	if (ToolTipClass)
	{
		UInventoryToolTip* ToolTip = CreateWidget<UInventoryToolTip>(this, ToolTipClass);
		if (ToolTip)
		{
			ToolTip->InventorySlotBeingHovered = this;
			SetToolTip(ToolTip);
		}
	}		
	else
	{
		SetToolTip(nullptr); // ºó ½½·ÔÀÌ¸é ÅøÆÁ Á¦°Å
	}

	RefreshSlot();
}

void UInventoryItemSlot::RefreshSlot()
{

	if (ItemReference)
	{
		ItemIcon->SetBrushFromTexture(ItemReference->AssetData.ItemImage);
		ItemIcon->SetVisibility(ESlateVisibility::Visible);

		if (ItemReference->NumericData.bIsStackable)
		{
			ItemQuantity->SetText(FText::AsNumber(ItemReference->Quantity));
			ItemQuantity->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		ItemIcon->SetVisibility(ESlateVisibility::Collapsed);
		ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
	}
}

FReply UInventoryItemSlot::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	FReply Reply = Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if(bShowToolTip)
			return Reply.Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		else
			return Reply.Unhandled();
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (AArcher* Player = Cast<AArcher>(GetOwningPlayerPawn()))
		{
			ItemReference->Use(Player);
		}
		return Reply.Handled();
	}

	return Reply.Unhandled();
}

void UInventoryItemSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
}

void UInventoryItemSlot::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	if (!ItemReference)
	{
		return;
	}

	if (DragItemVisualClass)
	{
		const TObjectPtr<UDragItemVisual> DragVisual = CreateWidget<UDragItemVisual>(this, DragItemVisualClass);
		DragVisual->ItemIcon->SetBrushFromTexture(ItemReference->AssetData.ItemImage);
		
		if (ItemReference->NumericData.bIsStackable)
		{
			DragVisual->ItemQuantity->SetText(FText::AsNumber(ItemReference->Quantity));
		}
		else
		{
			DragVisual->ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
		}

		UItemDragDropOperation* DragItemOperation = NewObject<UItemDragDropOperation>();
		DragItemOperation->SourceItem = ItemReference;
		DragItemOperation->SourceInventory = ItemReference->OwningInventory;
		DragItemOperation->SourceInventoryIndex = SlotIndex;
		DragItemOperation->bFromHotbar = false;
		DragItemOperation->SourceHotbarIndex = -1;

		DragItemOperation->DefaultDragVisual = DragVisual;
		DragItemOperation->Pivot = EDragPivot::TopLeft;

		OutOperation = DragItemOperation;
	}
}

bool UInventoryItemSlot::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UItemDragDropOperation* DragOp = Cast<UItemDragDropOperation>(InOperation);

	if (!DragOp || !DragOp->SourceInventory)
	{
		return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	}

	if (DragOp->bFromHotbar)
	{
		return DragOp->SourceInventory->MoveItemFromHotbarToInventorySlot(DragOp->SourceHotbarIndex, SlotIndex);
	}
	else
	{
		return DragOp->SourceInventory->MoveInventoryItem(DragOp->SourceInventoryIndex, SlotIndex);
	}
}
