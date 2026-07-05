// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/Inventory/InventoryPannel.h"
#include "Character/Archer.h"
#include "Inventory/InventoryComponent.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"
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
    Super::NativeConstruct();
    RefreshInventory();
}

void UInventoryPannel::SetInfoText() const
{
    CapacityInfo->SetText(FText::Format(FText::FromString("{0}/{1}"), InventoryReference->GetUsedSlotCount(), InventoryReference->GetSlotsCapacity()));
}

void UInventoryPannel::RefreshInventory()
{
    if (InventoryReference && InventorySlotClass)
    {
        InventoryPannel->ClearChildren();

        const int32 Capacity = InventoryReference->GetSlotsCapacity();

        for (int32 i = 0; i < Capacity; i++)
        {
            const FInventoryItemInstance Item = InventoryReference->GetInventoryItem(i);

            UInventoryItemSlot* ItemSlot = CreateWidget<UInventoryItemSlot>(this, InventorySlotClass);
            ItemSlot->SlotIndex = i;
            ItemSlot->SetOwningInventory(InventoryReference);
            ItemSlot->SetOwningInventoryIndex(i);
            ItemSlot->SetItem(Item);
            ItemSlot->bShowToolTip = Item.IsValidItem();
            ItemSlot->RefreshSlot();

            InventoryPannel->AddChildToWrapBox(ItemSlot);
        }

        RefreshHotbar();
        SetInfoText();
    }
}

void UInventoryPannel::RefreshHotbar()
{
    if (HotbarSlotClass)
    {
        HotbarBox->ClearChildren();
        HotbarSlots.Empty();

        for (int32 i = 0; i < HotbarSlotCount; i++)
        {
            UHotBarSlot* HotBarSlot = CreateWidget<UHotBarSlot>(this, HotbarSlotClass);
            HotBarSlot->SlotIndex = i;
            HotBarSlot->HotKeyNumber->SetText(FText::AsNumber(i + 1));

            const FInventoryItemInstance HotbarItem = InventoryReference->GetHotbarItem(i);

            HotBarSlot->SetOwningInventory(InventoryReference);
            HotBarSlot->SetItem(HotbarItem);
            HotBarSlot->bShowToolTip = HotbarItem.IsValidItem();
            HotBarSlot->RefreshSlot();

            HotbarBox->AddChildToHorizontalBox(HotBarSlot);
            HotbarSlots.Add(HotBarSlot);
        }
    }
}

bool UInventoryPannel::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    return false;
}