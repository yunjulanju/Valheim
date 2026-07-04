// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/InventoryToolTip.h"
#include "Components/TextBlock.h"
#include "Data/ItemPrimaryDataAsset.h"
#include "UserInterface/Inventory/InventoryItemSlot.h"

void UInventoryToolTip::NativeConstruct()
{
	Super::NativeConstruct();

	if (!InventorySlotBeingHovered)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	const FInventoryItemInstance& ItemBeingHovered = InventorySlotBeingHovered->GetItem();
	UItemPrimaryDataAsset* ItemData = InventorySlotBeingHovered->GetDisplayItemData();

	if (ItemData)
	{
		//TextAsset
		ItemName->SetText(ItemData->TextData.Name);
		ItemCategory->SetText(FText::AsNumber(ItemBeingHovered.Quantity));
		ItemDescription->SetText(ItemData->TextData.Description);

		//NumericAsset
		if (ItemData->NumericData.bIsStackable)
		{
			MaxStackSize->SetText(FText::AsNumber(ItemData->NumericData.MaxStackSize));
		}
		else
		{
			MaxStackSize->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}
