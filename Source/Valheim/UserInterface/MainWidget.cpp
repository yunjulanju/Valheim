// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/MainWidget.h"
#include "Character/Archer.h"
#include "UserInterface/Inventory/ItemDragDropOperation.h"
#include "Item/ItemDataBase.h"

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

	if (PlayerCharacter && ItemDragDrop && ItemDragDrop->SourceItem)
	{
		PlayerCharacter->DropItem(ItemDragDrop->SourceItem, ItemDragDrop->SourceItem->Quantity);
		return true;
	}
	return false;
}
