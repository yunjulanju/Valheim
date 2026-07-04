// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/MainWidget.h"
#include "Character/Archer.h"
#include "UserInterface/Inventory/ItemDragDropOperation.h"
#include "Item/ItemDataBase.h"
#include "Inventory/InventoryComponent.h"

void UMainWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UMainWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerCharacter = Cast<AArcher>(GetOwningPlayerPawn());
}

bool UMainWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);

	if (PlayerCharacter && ItemDragDrop && ItemDragDrop->SourceInventoryIndex != INDEX_NONE)
	{
		const FInventoryItemInstance DroppedItem = PlayerCharacter->GetInventory()
			? PlayerCharacter->GetInventory()->GetInventoryItem(ItemDragDrop->SourceInventoryIndex)
			: FInventoryItemInstance();

		PlayerCharacter->DropItem(ItemDragDrop->SourceInventoryIndex, DroppedItem.Quantity);
		return true;
	}
	return false;
}