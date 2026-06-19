// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/InventoryPannel.h"
#include "Character/Archer.h"
#include "Inventory/InventoryComponent.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
#include "Item/ItemDataBase.h"
#include "UserInterface/Inventory/InventoryItemSlot.h"
#include "ItemDragDropOperation.h"
#include "Components/HorizontalBox.h"
#include "UserInterface/Inventory/HotBarSlot.h"

void UInventoryPannel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	PlayerCharacter = Cast<AArcher>(GetOwningPlayerPawn());
	if (PlayerCharacter)
	{
		InventoryReference = PlayerCharacter->GetInventory();
		if (InventoryReference)
		{
			InventoryReference->OnInventoryUpdated.AddUObject(this, &UInventoryPannel::RefreshInventory);
			SetInfoText();
		}
	}

}

void UInventoryPannel::NativeConstruct()
{
    RefreshInventory();
}

void UInventoryPannel::SetInfoText() const
{
	CapacityInfo->SetText(FText::Format(FText::FromString("{0}/{1}"), InventoryReference->GetInventoryContents().Num(), InventoryReference->GetSlotsCapacity()));
}

void UInventoryPannel::RefreshInventory()
{
    if (InventoryReference && InventorySlotClass)
    {
        InventoryPannel->ClearChildren();

        int32 FilledCount = 0;

        for (UItemDataBase* const& InventoryItem : InventoryReference->GetInventoryContents())
        {
            if (!InventoryItem || InventoryItem->Quantity <= 0) continue;

            UInventoryItemSlot* ItemSlot = CreateWidget<UInventoryItemSlot>(this, InventorySlotClass);
            ItemSlot->SetItemReference(InventoryItem);
            ItemSlot->bShowToolTip = true;
            InventoryPannel->AddChildToWrapBox(ItemSlot);
            FilledCount++;
        }

        // capacity¸¸Å­ ºó ½½·Ô Ã¤¿ì±â
        const int32 EmptySlotCount = InventoryReference->GetSlotsCapacity() - FilledCount;
        for (int32 i = 0; i < EmptySlotCount; i++)
        {
            UInventoryItemSlot* EmptySlot = CreateWidget<UInventoryItemSlot>(this, InventorySlotClass);
            EmptySlot->SetItemReference(nullptr);
            EmptySlot->bShowToolTip = false;
            InventoryPannel->AddChildToWrapBox(EmptySlot);
        }
        RefreshHotbar();
        SetInfoText();
    }
}

void UInventoryPannel::RefreshHotbar()
{
    if (InventorySlotClass)
    {
        HotbarBox->ClearChildren();
        HotbarSlots.Empty();

        for (int32 i = 0; i < HotbarSlotCount; i++)
        {
            UInventoryItemSlot* HotbarSlot = CreateWidget<UInventoryItemSlot>(this, InventorySlotClass);

            HotbarBox->AddChildToHorizontalBox(HotbarSlot);
            HotbarSlots.Add(HotbarSlot);

            UHotbarSlot* Slot = CreateWidget<UHotbarSlot>(this, HotbarSlotClass);
            Slot->SlotIndex = i;
            Slot->HotKeyNumber->SetText(FText::AsNumber(i + 1));

            UItemDataBase* HotbarItem = InventoryReference->GetHotbarItem(i);
            Slot->SetItemReference(HotbarItem);

            HotbarBox->AddChildToHorizontalBox(Slot);
            HotbarSlots.Add(Slot);
        }
    }
}

bool UInventoryPannel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	const UItemDragDropOperation* ItemDragDrop = Cast<UItemDragDropOperation>(InOperation);

	if (ItemDragDrop->SourceItem && InventoryReference)
	{
		return true;
	}

	return false;
}
