// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/InventoryItemSlot.h"
#include "UserInterface/Inventory/InventoryToolTip.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Data/ItemPrimaryDataAsset.h"
#include "Item/ItemSubsystem.h"
#include "UserInterface/Inventory/DragItemVisual.h"
#include "ItemDragDropOperation.h"
#include "Character/Archer.h"
#include "Inventory/InventoryComponent.h"
#include "Engine/GameInstance.h"


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
		SetToolTip(nullptr);
	}

	RefreshSlot();
}

UItemPrimaryDataAsset* UInventoryItemSlot::GetDisplayItemData() const
{
	if (!CurrentItem.IsValidItem())
	{
		return nullptr;
	}

	if (CurrentItem.CachedItemData)
	{
		return CurrentItem.CachedItemData;
	}

	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GI = World->GetGameInstance())
		{
			if (UItemSubsystem* Subsystem = GI->GetSubsystem<UItemSubsystem>())
			{
				UItemPrimaryDataAsset* Found = nullptr;
				Subsystem->GetItemData(CurrentItem.ItemID, Found);
				return Found;
			}
		}
	}

	return nullptr;
}

void UInventoryItemSlot::RefreshSlot()
{
	if (UItemPrimaryDataAsset* ItemData = GetDisplayItemData())
	{
		ItemIcon->SetBrushFromTexture(ItemData->AssetData.ItemImage);
		ItemIcon->SetVisibility(ESlateVisibility::Visible);

		if (ItemData->NumericData.bIsStackable)
		{
			ItemQuantity->SetText(FText::AsNumber(CurrentItem.Quantity));
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
		if (bShowToolTip)
			return Reply.Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
		else
			return Reply.Unhandled();
	}

	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		if (AArcher* Player = Cast<AArcher>(GetOwningPlayerPawn()))
		{
			Player->UseItem(OwningInventoryIndex);
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

	if (!CurrentItem.IsValidItem() || !OwningInventory)
	{
		return;
	}

	if (DragItemVisualClass)
	{
		UDragItemVisual* DragVisual = CreateWidget<UDragItemVisual>(this, DragItemVisualClass);

		if (UItemPrimaryDataAsset* ItemData = GetDisplayItemData())
		{
			DragVisual->ItemIcon->SetBrushFromTexture(ItemData->AssetData.ItemImage);

			if (ItemData->NumericData.bIsStackable)
			{
				DragVisual->ItemQuantity->SetText(FText::AsNumber(CurrentItem.Quantity));
			}
			else
			{
				DragVisual->ItemQuantity->SetVisibility(ESlateVisibility::Collapsed);
			}
		}

		UItemDragDropOperation* DragItemOperation = NewObject<UItemDragDropOperation>();
		DragItemOperation->SourceInventory = OwningInventory;
		DragItemOperation->SourceInventoryIndex = OwningInventoryIndex;
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
