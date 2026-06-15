// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/InventoryToolTip.h"
#include "Components/TextBlock.h"
#include "Item/ItemDataBase.h"
#include "UserInterface/Inventory/InventoryItemSlot.h"

void UInventoryToolTip::NativeConstruct()
{
	Super::NativeConstruct();

	UItemDataBase* ItemBeingHovered = InventorySlotBeingHovered->GetItemReference();

	//TextAsset
	ItemName->SetText(ItemBeingHovered->TextData.Name);
	ItemCategory->SetText(FText::AsNumber(ItemBeingHovered->Quantity));
	ItemDescription->SetText(ItemBeingHovered->TextData.Description);

	//NumericAsset
	if (ItemBeingHovered->NumericData.bIsStackable)
	{
		MaxStackSize->SetText(FText::AsNumber(ItemBeingHovered->NumericData.MaxStackSize));
	}
	else
	{
		MaxStackSize->SetVisibility(ESlateVisibility::Collapsed);
	}
}
